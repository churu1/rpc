#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rocket/net/rpc/rpc_dispatcher.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/common/log.h"
#include "rpc_dispatcher.h"


namespace rocket {

void RpcDispathcer::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response) {

  std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
  std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);

  std::string method_full_name = req_protocol->m_method_name;
  std::string service_name;
  std::string method_name;

  if (parseServiceFullName(method_full_name, service_name, method_name)) {
    // TODO: 后面补充
    rsp_protocol->m_err_code();
  }

  auto it = m_service_map.find(service_name);
  if (it == m_service_map.end()) {
    // TODO: 后面补充
  }

  service_ptr service = it->second;

  service->GetDescriptor()->FindMethodByName(method_name);

  const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
  if (method == NULL) {
    // TODO: 后面补充
  }

  google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();

  //  反序列化，将 pb_data 反序列化为 req_msg
  if (!req_msg->ParseFromString(req_protocol->m_pb_data)) {
    // TODO 失败处理
  }

  INFOLOG("req_id[%s], get rpc request[%s]", req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str());
  
  google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

  // 调用 RPC 方法
  service->CallMethod(method, NULL, req_msg, rsp_msg, NULL);

  rsp_protocol->m_req_id = request->m_req_id;
  rsp_protocol->m_method_name = req_protocol->m_method_name;
  rsp_protocol->m_err_code = 0;

  rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data));
}

void RpcDispathcer::registerService(service_ptr service) {
  std::string service_name = service->GetDescriptor()->full_name();
  m_service_map[service_name] = service;
}

void RpcDispathcer::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t error_code, const std::string err_info) {
  msg->m_err_code = error_code;
  msg->m_err_info = err_info;
  msg->m_err_info_len = err_info.length();
}

bool RpcDispathcer::parseServiceFullName(const std::string full_name, std::string &service_name, std::string &method_name) {
  if (full_name.empty()) {
    ERRORLOG("full name empty");
    return false;
  }
  
  size_t i = full_name.find_first_of(".");
  if (i == full_name.npos) {
    ERRORLOG("not find . in full name [%s]", full_name.c_str());
    return false;
  }

  service_name = full_name.substr(0, i);
  method_name = full_name.substr(i + 1, full_name.length() - i - 1);

  INFOLOG("parse service_name[%s] and method_name from full name[%s]", service_name.c_str(), method_name.c_str());

  return true;
}





}
