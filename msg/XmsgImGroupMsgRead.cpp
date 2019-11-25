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
#include "XmsgImGroupMsgRead.h"

XmsgImGroupMsgRead::XmsgImGroupMsgRead()
{

}

void XmsgImGroupMsgRead::handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupMsgReadReq> req)
{
	if (req->cgt().empty() || req->msgid() < 1)
	{
		trans->endDesc(RET_FORMAT_ERROR, "format error");
		return;
	}
	shared_ptr<XmsgImGroupMemberColl> gm = ul->inGroupInfo(req->cgt());
	if (gm == nullptr) 
	{
		trans->endDesc(RET_FORBIDDEN, "you are not in group");
		return;
	}
	SptrCgt gcgt = ChannelGlobalTitle::parse(req->cgt());
	if (gcgt == nullptr || !gcgt->isGroup())
	{
		trans->endDesc(RET_FORMAT_ERROR, "format error, cgt: %s", req->cgt().c_str());
		return;
	}
	SptrGroup group = XmsgImGroupMgr::instance()->findGroup(gcgt);
	if (group == nullptr)
	{
		trans->endDesc(RET_NOT_FOUND, "can not found group for cgt: %s", req->cgt().c_str());
		return;
	}
	group->future([ul, client, trans, req, group, gm] 
	{
		XmsgImGroupMsgRead::handle4groupLocal(ul, client, trans, req, static_pointer_cast<XmsgImGroupMemberGlocal>(group), gm);
	});
}

void XmsgImGroupMsgRead::handle4groupLocal(SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupMsgReadReq> req, SptrGl gl, shared_ptr<XmsgImGroupMemberColl> gm)
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
	if (req->msgid() > gl->getMsgIdCurrent()) 
	{
		trans->endDesc(RET_FORBIDDEN, "over the current max message id in group");
		return;
	}
	shared_ptr<XmsgImGroupMsgReadRsp> rsp(new XmsgImGroupMsgReadRsp());
	trans->end(rsp);
	ul->future([ul, client, req, gm]
	{
		XmsgImGroupMsgRead::updateLatestReadMsgId(ul, client, req, gm);
	});
}

void XmsgImGroupMsgRead::updateLatestReadMsgId(SptrUl ul, SptrClientLocal client, shared_ptr<XmsgImGroupMsgReadReq> req, shared_ptr<XmsgImGroupMemberColl> gm)
{
	gm->latestReadMsgId = req->msgid() > gm->latestReadMsgId ? req->msgid() : gm->latestReadMsgId; 
	gm->uts = DateMisc::nowGmt0();
	ullong latestReadMsgId = gm->latestReadMsgId;
	ullong uts = gm->uts;
	ullong sts = DateMisc::dida();
	XmsgImGroupDb::instance()->future([gm, latestReadMsgId, uts, sts]
	{
		if (!XmsgImGroupMemberCollOper::instance()->updateLatestReadMsgId(gm->gcgt, gm->mcgt, latestReadMsgId, uts))
		{
			LOG_ERROR("update group member collection failed, mcgt: %s, gcgt: %s, latestReadMsgId: %llu, elap: %dms", gm->mcgt->toString().c_str(), gm->gcgt->toString().c_str(), latestReadMsgId, DateMisc::elapDida(sts))
			return;
		}
		LOG_DEBUG("update group member collection successful, mcgt: %s, gcgt: %s, latestReadMsgId: %llu, elap: %dms", gm->mcgt->toString().c_str(), gm->gcgt->toString().c_str(), latestReadMsgId, DateMisc::elapDida(sts))
	}, gm->mcgt);
	shared_ptr<XmsgImGroupMsgReadNotice> notice(new XmsgImGroupMsgReadNotice());
	notice->set_cgt(req->cgt());
	notice->set_msgid(req->msgid());
	ul->pubNotice2onlineClient(req->cgt(), notice, client);
}

XmsgImGroupMsgRead::~XmsgImGroupMsgRead()
{

}

