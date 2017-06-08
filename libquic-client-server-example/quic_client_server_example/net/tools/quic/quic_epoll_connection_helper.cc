// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/tools/quic/quic_epoll_connection_helper.h"

#include <errno.h>
#include <sys/socket.h>

#include "base/logging.h"
#include "base/stl_util.h"
#include "net/base/ip_endpoint.h"
#include "net/quic/crypto/quic_random.h"
#include "net/tools/epoll_server/epoll_server.h"
#include "net/tools/quic/quic_socket_utils.h"

namespace net {
namespace tools {

//namespace { //yang add change  

//ע��QuicEpollAlarm(�̳�QuicAlarm)  EpollAlarmImpl(�̳�EpollAlarm(�̳�EpollAlarmCallbackInterface))�Ĺ�ϵ,����
//ͨ��QuicEpollAlarm::SetImpl()->EpollServer::RegisterAlarm()�ν�����
class QuicEpollAlarm : public QuicAlarm {  //�����EpollAlarmImpl���а���QuicEpollAlarm���Ա
 public:
 /*
   ���õ�alarm���£��ο�QuicConnection::QuicConnection
   ����ӿ���QuicEpollConnectionHelper::CreateAlarm��ֵ��ʵ�ָ������⼴����:
      AckAlarm  RetransmissionAlarm  SendAlarm  SendAlarm  TimeoutAlarm  PingAlarm  FecAlarm
   �����ÿ��alarm���ж�Ӧ������QuicEpollAlarm�࣬���������QuicEpollAlarm::QuicAlarm::Delegate�ֱ��ɸ��Ե�alarmʵ��
*/
  //QuicEpollConnectionHelper::CreateAlarm()��new QuicEpollAlarm��
  QuicEpollAlarm(EpollServer* epoll_server,
                 QuicAlarm::Delegate* delegate)
      : QuicAlarm(delegate),
        epoll_server_(epoll_server),
        epoll_alarm_impl_(this) {}

 protected:
  void SetImpl() override { //EpollServerע��
    DCHECK(deadline().IsInitialized());
    epoll_server_->RegisterAlarm(
        deadline().Subtract(QuicTime::Zero()).ToMicroseconds(),
        &epoll_alarm_impl_);
  }

  void CancelImpl() override {
    DCHECK(!deadline().IsInitialized());
    epoll_alarm_impl_.UnregisterIfRegistered();
  }

 private:
  //EpollAlarmImpl�����QuicEpollAlarm��Ա��QuicEpollAlarmҲ����EpollAlarmImpl���Ա
  class EpollAlarmImpl : public EpollAlarm { //EpollAlarmImpl epoll_alarm_impl_;
   public:
    explicit EpollAlarmImpl(QuicEpollAlarm* alarm) : alarm_(alarm) {}

    int64 OnAlarm() override {//�ӿں���ִ�м�CallAndReregisterAlarmEvents
      EpollAlarm::OnAlarm();
	  //AckAlarm  RetransmissionAlarm  SendAlarm  SendAlarm  TimeoutAlarm  PingAlarm  FecAlarm
	  //��Щ���fire�����ӿ�
      alarm_->Fire();
      // Fire will take care of registering the alarm, if needed.
      return 0;
    }

   private:
   	//AckAlarm  RetransmissionAlarm  SendAlarm  SendAlarm  TimeoutAlarm  PingAlarm  FecAlarm
    QuicEpollAlarm* alarm_;
  };

  //ͨ���ó�Ա��QuicEpollAlarm��EpollAlarm��ϵ��������QuicEpollAlarm::SetImpl()->EpollServer::RegisterAlarm()
  EpollServer* epoll_server_; 
  
  EpollAlarmImpl epoll_alarm_impl_;
};

//}  // namespace

QuicEpollConnectionHelper::QuicEpollConnectionHelper(EpollServer* epoll_server)
    : epoll_server_(epoll_server),
      clock_(epoll_server),
      random_generator_(QuicRandom::GetInstance()) {
}

QuicEpollConnectionHelper::~QuicEpollConnectionHelper() {
}

const QuicClock* QuicEpollConnectionHelper::GetClock() const {
  return &clock_;
}

QuicRandom* QuicEpollConnectionHelper::GetRandomGenerator() {
  return random_generator_;
}

/*
   ���õ�alarm���£��ο�QuicConnection::QuicConnection
      ack_alarm_(helper->CreateAlarm(new AckAlarm(this))),
      retransmission_alarm_(helper->CreateAlarm(new RetransmissionAlarm(this))),
      send_alarm_(helper->CreateAlarm(new SendAlarm(this))),
      resume_writes_alarm_(helper->CreateAlarm(new SendAlarm(this))),
      timeout_alarm_(helper->CreateAlarm(new TimeoutAlarm(this))),
      ping_alarm_(helper->CreateAlarm(new PingAlarm(this))),
      fec_alarm_(helper->CreateAlarm(new FecAlarm(&packet_generator_))),
*/
QuicAlarm* QuicEpollConnectionHelper::CreateAlarm(
    QuicAlarm::Delegate* delegate) { 
  return new QuicEpollAlarm(epoll_server_, delegate);
}

}  // namespace tools
}  // namespace net
