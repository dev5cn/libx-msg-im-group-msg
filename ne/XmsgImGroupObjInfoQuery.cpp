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

#include "XmsgImGroupObjInfoQuery.h"

XmsgImGroupObjInfoQuery::XmsgImGroupObjInfoQuery()
{

}

void XmsgImGroupObjInfoQuery::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImGroupObjInfoQueryReq> req)
{
	if (req->oid().empty())
	{
		LOG_DEBUG("oid format error, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "oid format error");
	}
	if (req->ucgt().empty())
	{
		LOG_DEBUG("usr channel global title format error, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "usr channel global title format error");
		return;
	}
	SptrCgt ucgt = ChannelGlobalTitle::parse(req->ucgt());
	if (ucgt == nullptr)
	{
		LOG_DEBUG("usr channel global title format error, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "usr channel global title format error");
		return;
	}
	if (req->gcgt().empty())
	{
		LOG_DEBUG("group channel global title format error, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "group channel global title format error");
		return;
	}
	SptrCgt gcgt = ChannelGlobalTitle::parse(req->gcgt());
	if (gcgt == nullptr)
	{
		LOG_DEBUG("group channel global title format error, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "group channel global title format error");
		return;
	}
	auto group = XmsgImGroupMgr::instance()->findGroup(gcgt);
	if (group == nullptr)
	{
		LOG_DEBUG("can not found group for cgt, gcgt: %s", gcgt->toString().c_str())
		trans->endDesc(RET_FORBIDDEN, "can not found group for cgt, gcgt: %s", gcgt->toString().c_str());
		return;
	}
	if (!group->isMember(ucgt))
	{
		LOG_DEBUG("usr not group member, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORBIDDEN, "usr not group member, cgt: %s, gcgt: %s", gcgt->toString().c_str());
		return;
	}
	auto objInfo = XmsgImGroupObjInfoMgr::instance()->get(gcgt, req->oid());
	if (objInfo == nullptr)
	{
		LOG_DEBUG("can not found object info, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_FORBIDDEN, "can not found object info, oid: %s", req->oid().c_str());
		return;
	}
	shared_ptr<XmsgImGroupObjInfoQueryRsp> rsp(new XmsgImGroupObjInfoQueryRsp());
	rsp->set_ucgt(objInfo->ucgt->toString());
	rsp->set_gts(objInfo->gts);
	trans->end(rsp);
}

XmsgImGroupObjInfoQuery::~XmsgImGroupObjInfoQuery()
{

}

