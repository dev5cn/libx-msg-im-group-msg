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
#include "XmsgImGroupSendMsg.h"
#include "SendMsgUsrLocal.h"

XmsgImGroupSendMsg::XmsgImGroupSendMsg()
{

}

void XmsgImGroupSendMsg::handle(shared_ptr<XmsgNeUsr> nu, SptrU u, SptrClient client, SptrXitp trans, SptrSendMsgReq req)
{
	SptrCgt dest = ChannelGlobalTitle::parse(req->cgt());
	if (dest == nullptr)
	{
		LOG_DEBUG("global channel title format error, client: %s, req: %s", client->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "global channel title format error, cgt: %s", req->cgt().c_str());
		return;
	}
	if (!req->has_msg())
	{
		LOG_DEBUG("message content can not be null, client: %s, req: %s", client->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "message content can not be null");
		return;
	}
	SendMsgUsrLocal::handle(nu, static_pointer_cast<XmsgImGroupMemberUlocal>(u), client, trans, req, dest);
}

XmsgImGroupSendMsg::~XmsgImGroupSendMsg()
{

}
