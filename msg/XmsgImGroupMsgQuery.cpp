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
#include "XmsgImGroupMsgQuery.h"

XmsgImGroupMsgQuery::XmsgImGroupMsgQuery()
{

}

void XmsgImGroupMsgQuery::handle(shared_ptr<XmsgNeUsr> nu, SptrU u, SptrClient client, SptrXitp trans, shared_ptr<XmsgImGroupMsgQueryReq> req)
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
	group->future([u, client, trans, req, group]
	{
		XmsgImGroupMsgQuery::handle4groupLocal(u, client, trans, req, static_pointer_cast<XmsgImGroupMemberGlocal>(group));
	});
}

void XmsgImGroupMsgQuery::handle4groupLocal(SptrU u, SptrClient client, SptrXitp trans, shared_ptr<XmsgImGroupMsgQueryReq> req, SptrGl gl)
{
	shared_ptr<XmsgImGroupMemberInfo> gmi = gl->findMember(u->cgt);
	if (gmi == nullptr) 
	{
		trans->endDesc(RET_FORBIDDEN, "you are not group member");
		return;
	}
	if (!gmi->enable) 
	{
		trans->endDesc(RET_FORBIDDEN, "you are disabled in group");
		return;
	}
	ullong sts = DateMisc::dida();
	XmsgImGroupDb::instance()->future([u, client, trans, req, gl, sts]
	{
		XmsgImGroupMsgQuery::queryFromDb(u, client, trans, req, gl, sts);
	});
}

void XmsgImGroupMsgQuery::queryFromDb(SptrU u, SptrClient client, SptrXitp trans, shared_ptr<XmsgImGroupMsgQueryReq> req, SptrGl gl, ullong sts)
{
	int pageSize = req->pagesize() < 1 ? 1 : (req->pagesize() > 0x100 ? 0x100 : req->pagesize());
	list<shared_ptr<XmsgImGroupMsgColl>> msg;
	if (!XmsgImGroupMsgCollOper::instance()->queryMsgByPage(gl->cgt, req->msgid(), req->before(), pageSize, msg))
	{
		LOG_ERROR("query group message failed, elap: %dms, client: %s, group-local: %s, req: %s", DateMisc::elapDida(sts), client->toString().c_str(), gl->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "may be database exception");
		return;
	}
	LOG_TRACE("query group message successful, elap: %dms, msg-size: %zu, req: %s", DateMisc::elapDida(sts), msg.size(), req->ShortDebugString().c_str())
	if (msg.empty())
	{
		trans->end(RET_NO_RECORD);
		return;
	}
	shared_ptr<XmsgImGroupMsgQueryRsp> rsp(new XmsgImGroupMsgQueryRsp());
	for (auto& it : msg)
	{
		auto item = rsp->add_msg();
		item->set_scgt(it->scgt->toString());
		item->set_msgid(it->msgId);
		item->mutable_msg()->CopyFrom(*it->msg);
		item->set_gts(it->gts);
	}
	trans->end(rsp);
}

XmsgImGroupMsgQuery::~XmsgImGroupMsgQuery()
{

}

