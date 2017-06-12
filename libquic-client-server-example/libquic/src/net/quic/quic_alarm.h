// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_QUIC_ALARM_H_
#define NET_QUIC_QUIC_ALARM_H_

#include "base/memory/scoped_ptr.h"
#include "net/base/net_export.h"
#include "net/quic/quic_time.h"

namespace net {

// Abstract class which represents an alarm which will go off at a
// scheduled time, and execute the |OnAlarm| method of the delegate.
// An alarm may be cancelled, in which case it may or may not be
// removed from the underlying scheduling system, but in either case
// the task will not be executed.

/*
���������һ��������������Ԥ����ʱ��ִ�и�ί�е�| onAlarm |������
���������ȡ��������������£������ܻ򲻿��ܴӵײ����ϵͳ��ɾ������
���κ�����£����񽫲��ᱻִ�С�
*/
//ע��QuicEpollAlarm(�̳�QuicAlarm)  EpollAlarmImpl(�̳�EpollAlarm(�̳�EpollAlarmCallbackInterface))�Ĺ�ϵ,����
//ͨ��QuicEpollAlarm::SetImpl()->EpollServer::RegisterAlarm()�ν�����   //QuicEpollAlarm�̳и���   
class NET_EXPORT_PRIVATE QuicAlarm {
 public:
  class NET_EXPORT_PRIVATE Delegate{ 
  /*
     ���õ�alarm���£��ο�QuicConnection::QuicConnection
     ����ӿ���QuicEpollConnectionHelper::CreateAlarm��ֵ��ʵ�ָ������⼴����:
        AckAlarm  RetransmissionAlarm  SendAlarm  SendAlarm  TimeoutAlarm  PingAlarm  FecAlarm
     ����ĸ���alarm��洢��QuicConnection�����³�Ա��:
        ack_alarm_  retransmission_alarm_  send_alarm_  resume_writes_alarm_  timeout_alarm_  ping_alarm_  fec_alarm_
     �����ÿ��alarm���ж�Ӧ������QuicEpollAlarm�࣬���������QuicEpollAlarm::QuicAlarm::Delegate�ֱ��ɸ��Ե�alarmʵ��
  */
   public:
    virtual ~Delegate() {}

    // Invoked when the alarm fires.  If the return value is not
    // infinite, then the alarm will be rescheduled at the
    // specified time.
    virtual QuicTime OnAlarm() = 0;
  };

  explicit QuicAlarm(Delegate* delegate);
  virtual ~QuicAlarm();

  // Sets the alarm to fire at |deadline|.  Must not be called while
  // the alarm is set.  To reschedule an alarm, call Cancel() first,
  // then Set().
  void Set(QuicTime deadline); //����deadline_ʱ�䣬Ȼ��ִ��SetImpl

  // Cancels the alarm.  May be called repeatedly.  Does not
  // guarantee that the underlying scheduling system will remove
  // the alarm's associated task, but guarantees that the
  // delegates OnAlarm method will not be called.
  void Cancel();

  // Cancels and sets the alarm if the |deadline| is farther from the current
  // deadline than |granularity|, and otherwise does nothing.  If |deadline| is
  // not initialized, the alarm is cancelled.
  void Update(QuicTime deadline, QuicTime::Delta granularity); //����deadline_ʱ�� 

  bool IsSet() const;

  QuicTime deadline() const { return deadline_; }

 protected:
  // Subclasses implement this method to perform the platform-specific
  // scheduling of the alarm.  Is called from Set() or Fire(), after the
  // deadline has been updated.
  /*
  ����ʵ�ִ˷�����ִ��ƽ̨�ض��ı������ȡ�deadline_���ú�ͨ��set��fire�ӿڵ���SetImpl
  */
  //����epollע��
  virtual void SetImpl() = 0; //ʵ�ּ�QuicEpollAlarm::SetImpl��ע��epoll_server_->RegisterAlarm

  // Subclasses implement this method to perform the platform-specific
  // cancelation of the alarm.
  virtual void CancelImpl() = 0; //ʵ�ּ�QuicEpollAlarm::CancelImpl

  // Called by subclasses when the alarm fires.  Invokes the
  // delegates |OnAlarm| if a delegate is set, and if the deadline
  // has been exceeded.  Implementations which do not remove the
  // alarm from the underlying scheduler on Cancel() may need to handle
  // the situation where the task executes before the deadline has been
  // reached, in which case they need to reschedule the task and must not
  // call invoke this method.
  void Fire();

 private:
 /*
     ���õ�alarm���£��ο�QuicConnection::QuicConnection
     ����ӿ���QuicEpollConnectionHelper::CreateAlarm��ֵ��ʵ�ָ������⼴����:
        AckAlarm  RetransmissionAlarm  SendAlarm  SendAlarm  TimeoutAlarm  PingAlarm  FecAlarm
     �����ÿ��alarm���ж�Ӧ������QuicEpollAlarm�࣬���������QuicEpollAlarm::QuicAlarm::Delegate�ֱ��ɸ��Ե�alarmʵ��
  */
  scoped_ptr<Delegate> delegate_; //��ֵ��QuicEpollConnectionHelper::CreateAlarm()->QuicEpollAlarm()���캯���е���
  QuicTime deadline_; //Set��Update�ӿڸ�ֵ�͸���

  DISALLOW_COPY_AND_ASSIGN(QuicAlarm);
};

}  // namespace net

#endif  // NET_QUIC_QUIC_ALARM_H_

