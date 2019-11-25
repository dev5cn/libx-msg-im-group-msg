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

#include "XmsgImGroupSync.h"

XmsgImGroupSync::XmsgImGroupSync()
{

}

void XmsgImGroupSync::handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupSyncReq> req)
{
	if (req->ver4usr() == 0)
		XmsgImGroupSync::syncAll(nu, ul, client, trans, req);
	else
		XmsgImGroupSync::syncInc(nu, ul, client, trans, req);
	client->subGroupEvent = true;
}

void XmsgImGroupSync::syncAll(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupSyncReq> req)
{
	list<shared_ptr<XmsgImGroupMemberColl>> lis;
	for (auto& it : ul->group)
	{
		shared_ptr<XmsgImGroupMemberColl> coll = it.second;
		if (!coll->enable) 
			continue;
		lis.push_back(coll);
	}
	if (lis.empty()) 
	{
		trans->endDesc(RET_NO_RECORD, "you have no group");
		return;
	}
	lis.sort([](auto a, auto b)
	{
		return a->ver < b->ver; 
	});
	shared_ptr<XmsgImGroupSyncRsp> rsp(new XmsgImGroupSyncRsp());
	for (auto& it : lis)
	{
		XmsgImGroupSyncEvent* evn = rsp->add_event();
		evn->set_cgt(it->gcgt->toString());
		evn->set_oper("insert");
		SptrGroup g = XmsgImGroupMgr::instance()->findGroup(it->gcgt);
		if (g == nullptr)
		{
			LOG_FAULT("it`s a bug, can not found group for cgt: %s", it->gcgt->toString().c_str())
			continue;
		}
		*(evn->mutable_info()) = g->dat->info->kv(); 
		evn->set_ver4usr(it->ver);
		evn->set_gts4usr(it->gts);
		evn->set_uts4usr(it->uts);
		evn->set_gts4group(g->dat->gts);
	}
	trans->end(rsp);
}

void XmsgImGroupSync::syncInc(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupSyncReq> req)
{
	list<shared_ptr<XmsgImGroupMemberColl>> lis;
	for (auto& it : ul->group)
	{
		shared_ptr<XmsgImGroupMemberColl> coll = it.second;
		if (coll->ver <= req->ver4usr()) 
			continue;
		lis.push_back(coll);
	}
	if (lis.empty()) 
	{
		trans->end(RET_NOT_UPDATE);
		return;
	}
	lis.sort([](auto a, auto b)
	{
		return a->ver < b->ver; 
	});
	shared_ptr<XmsgImGroupSyncRsp> rsp(new XmsgImGroupSyncRsp());
	for (auto& it : lis)
	{
		XmsgImGroupSyncEvent* evn = rsp->add_event();
		evn->set_cgt(it->gcgt->toString());
		evn->set_oper(it->enable ? "insert" : "delete");
		SptrGroup g = XmsgImGroupMgr::instance()->findGroup(it->gcgt);
		if (g == nullptr)
		{
			LOG_FAULT("it`s a bug, can not found group for cgt: %s", it->gcgt->toString().c_str())
			continue;
		}
		if (it->enable)
			*(evn->mutable_info()) = g->dat->info->kv();
		evn->set_ver4usr(it->ver);
		evn->set_gts4usr(it->gts);
		evn->set_uts4usr(it->uts);
		evn->set_gts4group(g->dat->gts);
	}
	trans->end(rsp);
}

XmsgImGroupSync::~XmsgImGroupSync()
{

}

