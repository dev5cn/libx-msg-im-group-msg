/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <libx-msg-im-group-db.h>
#include "XmsgImGroupInfoUpdate.h"

XmsgImGroupInfoUpdate::XmsgImGroupInfoUpdate()
{

}

void XmsgImGroupInfoUpdate::handle(shared_ptr<XmsgNeUsr> nu, SptrU u, SptrClient client, SptrXitp trans, shared_ptr<XmsgImGroupInfoUpdateReq> req)
{
	if (req->cgt().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title can not be null");
		return;
	}
	SptrCgt gcgt = ChannelGlobalTitle::parse(req->cgt());
	if (gcgt == nullptr)
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title: %s", req->cgt().c_str());
		return;
	}
	if (req->upsert().empty() && req->remove().empty())
	{
		trans->endDesc(RET_FORBIDDEN, "update what?");
		return;
	}
	if (XmsgImGroupInfoUpdate::isRemoveSpecKey4groupInfo(req))
	{
		trans->endDesc(RET_FORBIDDEN, "update read-only field");
		return;
	}
	if (!gcgt->isGroup())
	{
		trans->endDesc(RET_FORBIDDEN, "channel global title must be a group: %s", req->cgt().c_str());
		return;
	}
	SptrGroup group = XmsgImGroupMgr::instance()->findGroup(gcgt);
	if (group == nullptr)
	{
		trans->endDesc(RET_FORBIDDEN, "can not found group for channel global title: %s", req->cgt().c_str());
		return;
	}
	group->future([u, client, trans, req, group]
	{
		if (!group->dat->enable)
		{
			trans->endDesc(RET_FORBIDDEN, "group disabled");
			return;
		}
		auto gmi = group->isMember(u->cgt);
		if (gmi == nullptr)
		{
			trans->endDesc(RET_FORBIDDEN, "you are not in group");
			return;
		}
		if(!gmi->havePermission2modifyGroupInfo()) 
		{
			trans->endDesc(RET_NO_PERMISSION, "you are not manager");
			return;
		}
		XmsgImGroupInfoUpdate::updateGroupInfo4local(u, client, trans, req, group);
	});
}

void XmsgImGroupInfoUpdate::updateGroupInfo4local(SptrU u, SptrClient client, SptrXitp trans, shared_ptr<XmsgImGroupInfoUpdateReq> req, SptrGroup group)
{
	XmsgMisc::updateKv(req->upsert(), req->remove(), *(group->dat->info->mutable_kv()));
	group->dat->ver = XmsgImGroupColl::version.fetch_add(1);
	group->dat->uts = DateMisc::nowGmt0();
	shared_ptr<XmsgKv> info(new XmsgKv());
	info->CopyFrom(*group->dat->info);
	ullong ver = group->dat->ver;
	ullong uts = group->dat->uts;
	ullong sts = DateMisc::dida();
	XmsgImGroupDb::instance()->future([trans, group, info, ver, uts, sts]
	{
		if (!XmsgImGroupCollOper::instance()->update(group->cgt, info, ver, uts))
		{
			trans->endDesc(RET_EXCEPTION, "can not update to database, cgt: %s", group->cgt->toString().c_str());
			return;
		}
		LOG_DEBUG("update group info to database successful, cgt: %s, ver: %llu, elap: %dms, info: %s", group->cgt->toString().c_str(), ver, DateMisc::elapDida(sts), info->ShortDebugString().c_str())
		trans->success();
	}, group->cgt);
}

bool XmsgImGroupInfoUpdate::isRemoveSpecKey4groupInfo(shared_ptr<XmsgImGroupInfoUpdateReq> req)
{
	for (int i = 0; i < req->remove_size(); ++i)
	{
		if ("private" != req->remove(i)) 
			continue;
		return true;
	}
	return false;
}

XmsgImGroupInfoUpdate::~XmsgImGroupInfoUpdate()
{

}

