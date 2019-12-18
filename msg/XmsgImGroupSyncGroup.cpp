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

#include "XmsgImGroupSyncGroup.h"

XmsgImGroupSyncGroup::XmsgImGroupSyncGroup()
{

}

void XmsgImGroupSyncGroup::handle(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupSyncGroupReq> req)
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
	if (!gcgt->isGroup()) 
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title must be a group: %s", req->cgt().c_str());
		return;
	}
	shared_ptr<XmsgImGroupMemberColl> gm = ul->inGroupInfo(req->cgt());
	if (gm == nullptr) 
	{
		trans->endDesc(RET_FORBIDDEN, "you are not in group");
		return;
	}
	SptrGroup group = XmsgImGroupMgr::instance()->findGroup(req->cgt());
	if (group == nullptr)
	{
		trans->endDesc(RET_NOT_FOUND, "can not found group for cgt: %s", req->cgt().c_str());
		return;
	}
	group->future([nu, ul, client, trans, req, group]
	{
		if (!group->dat->enable)
		{
			trans->endDesc(RET_FORBIDDEN, "group disabled");
			return;
		}
		if (group->isMember(ul->cgt) == nullptr) 
		{
			trans->endDesc(RET_FORBIDDEN, "can not found you in group");
			return;
		}
		if (req->ver() < 1)
		{
			XmsgImGroupSyncGroup::syncAll(nu, ul, client, trans, req, group);
			group->subEvn(client); 
			LOG_DEBUG("have a group member subscribe event successful, group: %s, member-client: %s", group->cgt->toString().c_str(), client->toString().c_str())
			return;
		}
		XmsgImGroupSyncGroup::syncInc(nu, ul, client, trans, req, group);
		group->subEvn(client); 
		LOG_DEBUG("have a group member subscribe event successful, group: %s, member-client: %s", group->cgt->toString().c_str(), client->toString().c_str())
	});
}

void XmsgImGroupSyncGroup::syncAll(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupSyncGroupReq> req, SptrGroup g)
{
	shared_ptr<XmsgImGroupSyncGroupRsp> rsp(new XmsgImGroupSyncGroupRsp());
	XmsgImGroupSyncGroupEventInfo* info = rsp->mutable_info();
	*(info->mutable_info()) = g->dat->info->kv();
	info->set_ver(g->dat->ver);
	info->set_gts(g->dat->gts);
	info->set_uts(g->dat->uts);
	list<shared_ptr<XmsgImGroupMemberInfo>> lis;
	for (auto& it : g->member)
	{
		auto m = it.second;
		if (!m->enable)
			continue;
		lis.push_back(m);
	}
	if (lis.empty()) 
	{
		trans->endDesc(RET_NO_RECORD, "group have no member");
		return;
	}
	lis.sort([](auto a, auto b)
	{
		return a->ver < b->ver; 
	});
	for (auto& it : lis)
	{
		XmsgImGroupSyncGroupEventMember* em = rsp->add_member();
		em->set_cgt(it->cgt->toString());
		em->set_oper("upsert");
		*(em->mutable_info()) = it->info->kv();
		em->set_ver(it->ver);
		em->set_gts(it->gts);
		em->set_uts(it->uts);
	}
	trans->end(rsp);
}

void XmsgImGroupSyncGroup::syncInc(shared_ptr<XmsgNeUsr> nu, SptrUl ul, SptrClientLocal client, SptrXitp trans, shared_ptr<XmsgImGroupSyncGroupReq> req, SptrGroup g)
{
	shared_ptr<XmsgImGroupSyncGroupRsp> rsp(new XmsgImGroupSyncGroupRsp());
	if (req->ver() < g->dat->ver)
	{
		XmsgImGroupSyncGroupEventInfo* info = rsp->mutable_info();
		*(info->mutable_info()) = g->dat->info->kv();
		info->set_ver(g->dat->ver);
		info->set_gts(g->dat->gts);
		info->set_uts(g->dat->uts);
	}
	list<shared_ptr<XmsgImGroupMemberInfo>> lis;
	for (auto& it : g->member)
	{
		auto m = it.second;
		if (m->ver <= req->ver())
			continue;
		lis.push_back(m);
	}
	if (lis.empty())
	{
		if (rsp->has_info())
			trans->end(rsp);
		else
			trans->end(RET_NOT_UPDATE); 
		return;
	}
	lis.sort([](auto a, auto b)
	{
		return a->ver < b->ver; 
	});
	for (auto& it : lis)
	{
		XmsgImGroupSyncGroupEventMember* em = rsp->add_member();
		em->set_cgt(it->cgt->toString());
		em->set_oper(it->enable ? "upsert" : "delete");
		*(em->mutable_info()) = it->info->kv();
		em->set_ver(it->ver);
		em->set_gts(it->gts);
		em->set_uts(it->uts);
	}
	trans->end(rsp);
}

XmsgImGroupSyncGroup::~XmsgImGroupSyncGroup()
{

}

