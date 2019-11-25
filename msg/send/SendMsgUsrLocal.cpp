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
#include "SendMsgUsrLocal.h"

SendMsgUsrLocal::SendMsgUsrLocal()
{

}

void SendMsgUsrLocal::handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest)
{
	if (!SendMsgUsrLocal::checkPermission4msgType(ul, client, req))
	{
		LOG_DEBUG("you don`t have permission send this message, client: %s, req: %s", client->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_NO_PERMISSION, "you don`t have permission send this message");
		return;
	}
	if (!SendMsgUsrLocal::checkPermission4dest(ul, client, req, dest))
	{
		LOG_DEBUG("you don`t have permission send this message to destination, client: %s, req: %s", client->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_NO_PERMISSION, "you don`t have permission send this message to destination: %s", req->cgt().c_str());
		return;
	}
	if (dest->isUsr())
	{
		SendMsgUsrLocal::msg4usr(ul, client, trans, req, dest);
		return;
	}
	SendMsgUsrLocal::msg4group(ul, client, trans, req, dest);
}

bool SendMsgUsrLocal::checkPermission4msgType(SptrUl org, SptrClient client, SptrSendMsgReq req)
{
	return true;
}

bool SendMsgUsrLocal::checkPermission4dest(SptrUl org, SptrClient client, SptrSendMsgReq req, SptrCgt dest)
{
	return true;
}

void SendMsgUsrLocal::msg4usr(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest)
{
	SendMsgUsrLocal::msg4usrLocal(org, client, trans, req, dest);
}

void SendMsgUsrLocal::msg4usrLocal(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest)
{
	SptrUl destUsrDat = static_pointer_cast<XmsgImGroupMemberUlocal>(XmsgImGroupMgr::instance()->findUsr(dest));
	if (destUsrDat == nullptr) 
	{
		LOG_DEBUG("can not found local destination usr, client: %s, req: %s", client->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_FORBIDDEN, "can not found local destination usr for cgt: %s", req->cgt().c_str());
		return;
	}
	string key = XmsgImGroupMgr::instance()->genKey(org->cgt, dest); 
	auto gl = static_pointer_cast<XmsgImGroupMemberGlocal>(XmsgImGroupMgr::instance()->findUsrToUsr(key));
	if (gl != nullptr) 
	{
		SendMsgUsrLocal::msgReceived(org, client, trans, req, destUsrDat->cgt, gl); 
		return;
	}
	ullong sts = DateMisc::dida();
	XmsgImGroupDb::instance()->future([org, client, trans, req, destUsrDat, key, sts] 
	{
		SendMsgUsrLocal::msg4usrLocal4createGroupLocal(org, client, trans, req, destUsrDat, key, sts);
	}, org->cgt);
}

void SendMsgUsrLocal::msg4usrLocal4createGroupLocal(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrUl dest, const string& key, ullong sts)
{
	auto gl = static_pointer_cast<XmsgImGroupMemberGlocal>(XmsgImGroupMgr::instance()->findUsrToUsr(key)); 
	if (gl != nullptr) 
	{
		SendMsgUsrLocal::msgReceived(org, client, trans, req, dest->cgt, gl); 
		return;
	}
	shared_ptr<XmsgImGroupUsrToUsrColl> coll(new XmsgImGroupUsrToUsrColl());
	coll->key = key;
	coll->gcgt = XmsgImGroupMgr::instance()->genGcgt();
	coll->u0 = org->cgt;
	coll->u1 = dest->cgt;
	coll->gts = DateMisc::nowGmt0();
	gl = XmsgImGroupUsrToUsrCollOper::instance()->createLocalGroup4localUsr(org, dest, coll); 
	if (gl == nullptr)
	{
		LOG_ERROR("create local group for local usr failed, coll: %s, elap: %dms", coll->toString().c_str(), DateMisc::elapDida(sts))
		trans->endDesc(RET_EXCEPTION, "can not create group for usr");
		return;
	}
	LOG_INFO("create a new local group for local usr successful, and we will notify to all member, coll: %s, elap: %dms", coll->toString().c_str(), DateMisc::elapDida(sts))
	gl->future([gl, org, client, trans, req, coll]
	{
		gl->pubEvnGroupCreateUsrToUsr(client, [gl, org, client, trans, req, coll]
				{
					SendMsgUsrLocal::msgReceived(org, client, trans, req, coll->u1, gl); 
				}); 
	});
}

void SendMsgUsrLocal::msg4group(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest)
{
	SendMsgUsrLocal::msg4groupLocal(org, client, trans, req, dest);
}

void SendMsgUsrLocal::msg4groupLocal(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest)
{
	auto gl = static_pointer_cast<XmsgImGroupMemberGlocal>(XmsgImGroupMgr::instance()->findGroup(req->cgt()));
	if (gl == nullptr)
	{
		trans->endDesc(RET_FORBIDDEN, "can not found destination group for cgt: %s", req->cgt().c_str());
		return;
	}
	SendMsgUsrLocal::msgReceived(org, client, trans, req, dest, gl);
}

void SendMsgUsrLocal::msgReceived(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl)
{
	if (XmsgImGroupCfg::instance()->cfgPb->misc().receivedmode() == X_MSG_IM_GROUP_MSG_RECEIVED_MODE_RECEIVED) 
	{
		SendMsgUsrLocal::msgReceivedInProcess(org, client, trans, req, dest, gl);
		return;
	}
	if (XmsgImGroupCfg::instance()->cfgPb->misc().receivedmode() == X_MSG_IM_GROUP_MSG_RECEIVED_MODE_DB_RECEIVED) 
	{
		SendMsgUsrLocal::msgReceivedDbReceived(org, client, trans, req, dest, gl);
		return;
	}
	if (XmsgImGroupCfg::instance()->cfgPb->misc().receivedmode() == X_MSG_IM_GROUP_MSG_RECEIVED_MODE_DB_DISK) 
	{
		SendMsgUsrLocal::msgReceivedDbDisk(org, client, trans, req, dest, gl);
		return;
	}
	LOG_FAULT("it`s a bug, received-mode: %02X", XmsgImGroupCfg::instance()->cfgPb->misc().receivedmode())
	trans->endDesc(RET_EXCEPTION, "system exception");
}

void SendMsgUsrLocal::msgReceivedInProcess(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl)
{
	SptrGroupMsg msg(new XmsgImGroupMsgColl());
	msg->scgt = org->cgt;
	msg->gcgt = gl->cgt;
	msg->msgId = gl->getMsgId();
	if (msg->msgId < 1)
	{
		LOG_FAULT("it`s a bug, can not generate new msgid for message, client: %s, req: %s, gl: %s", client->toString().c_str(), req->ShortDebugString().c_str(), gl->toString().c_str())
		trans->endDesc(RET_EXCEPTION, "can not generate new msgid for message");
		return;
	}
	msg->localMsgId = req->localmsgid();
	msg->msg.reset(new XmsgImMsg());
	msg->msg->CopyFrom(req->msg());
	msg->gts = DateMisc::nowGmt0();
	shared_ptr<XmsgImGroupSendMsgRsp> rsp(new XmsgImGroupSendMsgRsp());
	rsp->set_msgid(msg->msgId);
	if (dest->isUsr())
		rsp->set_gcgt(gl->cgt->toString());
	rsp->set_gts(msg->gts);
	trans->end(rsp);
	shared_ptr<XmsgImGroupMsgNotice> notice(new XmsgImGroupMsgNotice());
	notice->set_scgt(org->cgt->toString());
	notice->set_gcgt(gl->cgt->toString());
	notice->set_msgid(msg->msgId);
	notice->mutable_msg()->CopyFrom(req->msg());
	notice->set_gts(msg->gts);
	gl->future([org, client, trans, req, dest, gl, notice]
	{
		SendMsgUsrLocal::deliver(org, client, trans, req, dest, gl, notice);
	});
	ullong sts = DateMisc::dida();
	XmsgImGroupMsgCollOper::instance()->saveMsg(msg, [org, client, msg, sts](int ret, const string& desc)
	{
		if(ret != RET_SUCCESS)
		{
			LOG_ERROR("save message failed, elap: %dms, client: %s, msg: %s, ret: %08X, desc: %s", DateMisc::elapDida(sts), client->toString().c_str(), msg->toString().c_str(),ret, desc.c_str())
			return;
		}
		LOG_TRACE("save message successful, elap: %dms, client: %s, msg: %s", DateMisc::elapDida(sts), client->toString().c_str(), msg->toString().c_str())
	});
}

void SendMsgUsrLocal::msgReceivedDbReceived(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl)
{
	SptrGroupMsg msg(new XmsgImGroupMsgColl());
	msg->scgt = org->cgt;
	msg->gcgt = gl->cgt;
	msg->msgId = gl->getMsgId();
	if (msg->msgId < 1)
	{
		LOG_FAULT("it`s a bug, can not generate new msgid for message, client: %s, req: %s, gl: %s", client->toString().c_str(), req->ShortDebugString().c_str(), gl->toString().c_str())
		trans->endDesc(RET_EXCEPTION, "can not generate new msgid for message");
		return;
	}
	msg->localMsgId = req->localmsgid();
	msg->msg.reset(new XmsgImMsg());
	msg->msg->CopyFrom(req->msg());
	msg->gts = DateMisc::nowGmt0();
	ullong sts = DateMisc::dida();
	XmsgImGroupMsgCollOper::instance()->saveMsg(msg, [org, client, trans, req, dest, gl, msg, sts](int ret, const string& desc) 
	{
		if(ret != RET_SUCCESS)
		{
			LOG_ERROR("save message failed, ret: %08X, desc: %s, elap: %dms, client: %s, msg: %s", ret, desc.c_str(), DateMisc::elapDida(sts), client->toString().c_str(), msg->toString().c_str())
			trans->endDesc(RET_EXCEPTION, "can not save message to database");
			return;
		}
		LOG_TRACE("save message successful, elap: %dms, client: %s, msg: %s", DateMisc::elapDida(sts), client->toString().c_str(), msg->toString().c_str())
		shared_ptr<XmsgImGroupSendMsgRsp> rsp(new XmsgImGroupSendMsgRsp());
		rsp->set_msgid(msg->msgId);
		if (dest->isUsr())
		{
			rsp->set_gcgt(msg->gcgt->toString());
		}
		rsp->set_gts(DateMisc::nowGmt0());
		trans->end(rsp);
		shared_ptr<XmsgImGroupMsgNotice> notice(new XmsgImGroupMsgNotice());
		notice->set_scgt(org->cgt->toString());
		notice->set_gcgt(msg->gcgt->toString());
		notice->set_msgid(msg->msgId);
		notice->mutable_msg()->CopyFrom(req->msg());
		notice->set_gts(msg->gts);
		gl->future([org, client, trans, req, dest, gl, notice]
				{
					SendMsgUsrLocal::deliver(org, client, trans, req, dest, gl, notice);
				});
	});
}

void SendMsgUsrLocal::msgReceivedDbDisk(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl)
{
	SendMsgUsrLocal::msgReceivedDbReceived(org, client, trans, req, dest, gl);
}

void SendMsgUsrLocal::deliver(SptrUl org, SptrClient client, SptrXitp trans, SptrSendMsgReq req, SptrCgt dest, SptrGl gl, shared_ptr<XmsgImGroupMsgNotice> notice)
{
	gl->addMsgNotice(notice); 
	gl->pubMsgNotice2localUsr(notice, client); 
	gl->pubMsgNotice2localGroup(notice); 
}

SendMsgUsrLocal::~SendMsgUsrLocal()
{

}

