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

#include "XmsgImGroupInfoQuery.h"

XmsgImGroupInfoQuery::XmsgImGroupInfoQuery()
{

}

void XmsgImGroupInfoQuery::handle(shared_ptr<XmsgNeUsr> nu, SptrU u, SptrClient client, SptrXitp trans, shared_ptr<XmsgImGroupInfoQueryReq> req)
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
	group->future([u, group, trans, req]
	{
		XmsgImGroupInfoQuery::queryOnGroupThread(u, group, trans, req);
	});
}

void XmsgImGroupInfoQuery::queryOnGroupThread(SptrU u, SptrGroup group, SptrXitp trans, shared_ptr<XmsgImGroupInfoQueryReq> req)
{
	if (group->isMember(u->cgt) == nullptr)
	{
		trans->endDesc(RET_FORBIDDEN, "you are not in group");
		return;
	}
	shared_ptr<XmsgImGroupInfoQueryRsp> rsp(new XmsgImGroupInfoQueryRsp());
	rsp->set_allocated_info(group->dat->toSelfInfo());
	if (!req->member()) 
	{
		trans->end(rsp);
		return;
	}
	for (auto& it : group->member)
	{
		if (!it.second->enable) 
			continue;
		rsp->mutable_member()->AddAllocated(it.second->toMemberInfo());
	}
	trans->end(rsp);
}

XmsgImGroupInfoQuery::~XmsgImGroupInfoQuery()
{

}

