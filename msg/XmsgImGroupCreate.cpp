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
#include "XmsgImGroupCreate.h"

XmsgImGroupCreate::XmsgImGroupCreate()
{

}

void XmsgImGroupCreate::handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupCreateReq> req)
{
	if (req->member().empty())
	{
		LOG_DEBUG("can not create a group without member, ul: %s", ul->toString().c_str())
		trans->endDesc(RET_FORBIDDEN, "no member");
		return;
	}
	XmsgImGroupCreate::simple(nu, ul, client, trans, req);
}

void XmsgImGroupCreate::simple(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupCreateReq> req)
{
	if (req->member().size() + 1 > (int) XmsgImGroupCfg::instance()->cfgPb->misc().groupmembercountlimit())
	{
		LOG_DEBUG("group member count over limit, size: %d, ul: %s", XmsgImGroupCfg::instance()->cfgPb->misc().groupmembercountlimit(), ul->toString().c_str())
		trans->endDesc(RET_OVER_LIMIT, "group member count over limit, size: %d", XmsgImGroupCfg::instance()->cfgPb->misc().groupmembercountlimit());
		return;
	}
	shared_ptr<unordered_map<string, pair<SptrUl, shared_ptr<XmsgKv>>>> member(new unordered_map<string, pair<SptrUl, shared_ptr<XmsgKv>>>()); 
	for (auto& it : req->member())
	{
		SptrCgt mcgt = ChannelGlobalTitle::parse(it.cgt());
		if (mcgt == nullptr)
		{
			LOG_DEBUG("have some one channel global title format, ul: %s, cgt: %s", ul->toString().c_str(), it.cgt().c_str())
			trans->endDesc(RET_FORBIDDEN, "have some one channel global title format: %s", it.cgt().c_str());
			return;
		}
		auto m = XmsgImGroupMgr::instance()->findUsr(mcgt);
		if (m == nullptr) 
		{
			LOG_DEBUG("can not found usr for channel global title, ul: %s, cgt: %s", ul->toString().c_str(), mcgt->toString().c_str())
			trans->endDesc(RET_FORBIDDEN, "can not found usr for channel global title: %s", it.cgt().c_str());
			return;
		}
		if (m->isForeign())
		{
			LOG_DEBUG("can not create a group includes a foreign usr, ul: %s, cgt: %s", ul->toString().c_str(), mcgt->toString().c_str())
			trans->endDesc(RET_FORBIDDEN, "can not create a group includes a foreign usr: %s", it.cgt().c_str());
			return;
		}
		shared_ptr<XmsgKv> kv(new XmsgKv());
		*(kv->mutable_kv()) = it.info();
		(*member)[m->cgt->toString()] = make_pair<>(static_pointer_cast<XmsgImGroupMemberUlocal>(m), kv);
	}
	shared_ptr<XmsgKv> kv(new XmsgKv()); 
	(*member)[ul->cgt->toString()] = make_pair<>(ul, kv); 
	if (member->size() < 2) 
	{
		LOG_DEBUG("can not create a group only include your self, ul: %s", ul->toString().c_str())
		trans->endDesc(RET_FORBIDDEN, "can not create a group only include your self");
		return;
	}
	XmsgImGroupDb::instance()->future([ul, client, member, req, trans]
	{
		XmsgImGroupCreate::createGroupOnDbThread(ul, client, trans, req, member);
	}, ul->cgt);
}

void XmsgImGroupCreate::createGroupOnDbThread(SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupCreateReq> req, shared_ptr<unordered_map<string, pair<SptrUl, shared_ptr<XmsgKv>>>> member)
{
	SptrGl gl = XmsgImGroupCollOper::instance()->createGroupSimple(req, member, ul);
	if (gl == nullptr)
	{
		LOG_DEBUG("create local group failed, may be database exception, ul: %s, req: %s", ul->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "create local group failed, may be database exception");
		return;
	}
	shared_ptr<XmsgImGroupCreateRsp> rsp(new XmsgImGroupCreateRsp());
	rsp->set_cgt(gl->cgt->toString());
	rsp->set_gts(gl->dat->gts);
	trans->end(rsp);
	gl->future([gl, client]
	{
		gl->pubEvnGroupCreate(client, []
				{

				});
	});
}

XmsgImGroupCreate::~XmsgImGroupCreate()
{

}

