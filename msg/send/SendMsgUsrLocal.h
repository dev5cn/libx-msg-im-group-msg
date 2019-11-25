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

#ifndef MSG_SEND_SENDMSGUSRLOCAL_H_
#define MSG_SEND_SENDMSGUSRLOCAL_H_

#include "XmsgImGroupSendMsg.h"

class SendMsgUsrLocal
{
public:
	static void handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest); 
private:
	static bool checkPermission4msgType(SptrUl org, SptrClient client, SptrSendMsgReq req); 
	static bool checkPermission4dest(SptrUl org, SptrClient client, SptrSendMsgReq req, SptrCgt dest); 
private:
	static void msg4usr(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest); 
	static void msg4usrLocal(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest); 
	static void msg4usrLocal4createGroupLocal(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrUl dest, const string& key, ullong sts); 
private:
	static void msg4group(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest); 
	static void msg4groupLocal(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest); 
private:
	static void msgReceived(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl); 
	static void msgReceivedInProcess(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl); 
	static void msgReceivedDbReceived(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl); 
	static void msgReceivedDbDisk(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl); 
private:
	static void deliver(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl, shared_ptr<XmsgImGroupMsgNotice> notice); 
private:
	SendMsgUsrLocal();
	virtual ~SendMsgUsrLocal();
};

#endif 
