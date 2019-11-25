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

#include "XmsgImGroupMsgSync.h"

XmsgImGroupMsgSync::XmsgImGroupMsgSync()
{

}

void XmsgImGroupMsgSync::handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupMsgSyncReq> req)
{
	if (req->cgt().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title can not be null");
		return;
	}
	SptrCgt gcgt = ChannelGlobalTitle::parse(req->cgt());
	if (gcgt == nullptr)
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error: %s", req->cgt().c_str());
		return;
	}
	if (gcgt->isGroup()) 
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error: %s", req->cgt().c_str());
		return;
	}

	shared_ptr<XmsgImGroupMemberColl> gm = ul->inGroupInfo(req->cgt());
	if (gm == nullptr) 
	{
		trans->endDesc(RET_FORBIDDEN, "you are not in group");
		return;
	}
	SptrGl gl = static_pointer_cast<XmsgImGroupMemberGlocal>(XmsgImGroupMgr::instance()->findGroup(req->cgt()));
	if (gl == nullptr) 
	{
		trans->endDesc(RET_NOT_FOUND, "can not found group for channel global title: %s", req->cgt().c_str());
		return;
	}
	gl->future([ul, client, trans, req, gl, gm] 
	{
		if (gl->isMember(ul->cgt) == nullptr) 
		{
			trans->endDesc(RET_FORBIDDEN, "you are not in group");
			return;
		}
		XmsgImGroupMsgSync::handle4groupLocal(ul, client, trans, req, gl, gm);
	});
}

void XmsgImGroupMsgSync::handle4groupLocal(SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupMsgSyncReq> req, SptrGl gl, shared_ptr<XmsgImGroupMemberColl> gm)
{
	shared_ptr<XmsgImGroupMemberInfo> gmi = gl->findMember(ul->cgt);
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
	auto old = gl->subMsg(client).lock();
	if (old != nullptr)
	{
		LOG_WARN("duplicate subscriber group message and event, new: %s, old: %s", client->toString().c_str(), old->toString().c_str())
	}
	LOG_DEBUG("have a group member subscribe message successful, group: %s, member-client: %s", gl->cgt->toString().c_str(), client->toString().c_str())
	auto rsp = gl->makeXmsgImGroupMsgSyncRsp(req);
	if (rsp->msg().empty())
	{
		trans->end(RET_NO_RECORD); 
		return;
	}
	ul->future([ul, trans, gm, rsp]
	{
		for (int i = 0; i < rsp->msg_size(); ++i)
		{
			rsp->mutable_msg(i)->set_isread(rsp->msg(i).msgid() <= gm->latestReadMsgId); 
		}
		trans->end(rsp);
	});
}

XmsgImGroupMsgSync::~XmsgImGroupMsgSync()
{

}

