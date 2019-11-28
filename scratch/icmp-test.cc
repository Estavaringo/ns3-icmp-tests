#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simulator.h"
#include "ns3/icmpv6-header.h"
#include "ns3/icmpv4.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/internet-stack-helper.h"

#include "ns3/test.h"

#include <string>

NS_LOG_COMPONENT_DEFINE ("Icmpv4HeaderTest");

using namespace ns3;

/**
 * \ingroup internet-apps
 * \defgroup icmp-test ICMP protocol tests
 */


/**
 * \ingroup icmp-test
 * \ingroup tests
 *
 * \brief ICMP  Echo Reply Test
 */
class IcmpEchoReplyTestCase : public TestCase
{
public:
  IcmpEchoReplyTestCase ();
  virtual ~IcmpEchoReplyTestCase ();

  void SendData (Ptr<Socket> socket, Ipv4Address dst);
  void DoSendData (Ptr<Socket> socket, Ipv4Address dst);
  void ReceivePkt (Ptr<Socket> socket);

public:
  virtual void DoRun (void);
  Ptr<Packet> m_receivedPacket;

};


IcmpEchoReplyTestCase::IcmpEchoReplyTestCase ()
  : TestCase ("ICMP:EchoReply test case")
{

}


IcmpEchoReplyTestCase::~IcmpEchoReplyTestCase ()
{

}


void
IcmpEchoReplyTestCase::DoSendData (Ptr<Socket> socket, Ipv4Address dst)
{
  
  Ptr<Packet> p = Create<Packet> ();
  Icmpv4Echo echo;
  echo.SetSequenceNumber (1);
  echo.SetIdentifier (0);
  p->AddHeader (echo);
  printf("Enviando echo reply: \n");
  echo.Print(std::cout);
  printf("\n");

  Icmpv4Header header;
  header.SetType (Icmpv4Header::ICMPV4_ECHO);
  header.SetCode (0);
  p->AddHeader (header);
  
  header.Print(std::cout);

  Address realTo = InetSocketAddress (dst, 1234);

  NS_TEST_EXPECT_MSG_EQ (socket->SendTo (p, 0, realTo),
                         (int) p->GetSize (), " Unable to send ICMP Echo Packet");

}


void
IcmpEchoReplyTestCase::SendData (Ptr<Socket> socket, Ipv4Address dst)
{
  m_receivedPacket = Create<Packet> ();
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpEchoReplyTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpEchoReplyTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (0xffffffff, 0, from);
  printf("\n recebeu o pacote \n");
  
  m_receivedPacket = p->Copy ();

  Ipv4Header ipv4;
  p->RemoveHeader (ipv4);
  NS_TEST_EXPECT_MSG_EQ (ipv4.GetProtocol (), 1," The received Packet is not an ICMP packet");

  Icmpv4Header icmp;
  p->RemoveHeader (icmp);

  NS_TEST_EXPECT_MSG_EQ (icmp.GetType (), Icmpv4Header::ICMPV4_ECHO_REPLY,
                         " The received Packet is not a ICMPV4_ECHO_REPLY");
}


void
IcmpEchoReplyTestCase::DoRun ()
{

  printf("Iniciando IcmpEchoReplyTestCase... \n");
  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);

  // link the two nodes
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();
  n.Get (0)->AddDevice (txDev);
  n.Get (1)->AddDevice (rxDev);
  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
  rxDev->SetChannel (channel1);
  txDev->SetChannel (channel1);
  NetDeviceContainer d;
  d.Add (txDev);
  d.Add (rxDev);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.0.0.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign (d);

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (n.Get (0), TypeId::LookupByName ("ns3::Ipv4RawSocketFactory"));
  socket->SetAttribute ("Protocol", UintegerValue (1)); // ICMP protocol
  socket->SetRecvCallback (MakeCallback (&IcmpEchoReplyTestCase::ReceivePkt, this));

  InetSocketAddress src = InetSocketAddress (Ipv4Address::GetAny (), 0);
  NS_TEST_EXPECT_MSG_EQ (socket->Bind (src),0," Socket Binding failed");

  // Set a TTL big enough
  socket->SetIpTtl (1);
  SendData (socket, i.GetAddress (1,0));

  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 28, " Unexpected ICMPV4_ECHO_REPLY packet size");


  printf("Fim do IcmpEchoReplyTestCase... \n");
  Simulator::Destroy ();
}


/**
 * \ingroup icmp-test
 * \ingroup tests
 *
 * \brief ICMP Time Exceed Reply Test
 */
class IcmpTimeExceedTestCase : public TestCase
{
public:
  IcmpTimeExceedTestCase ();
  virtual ~IcmpTimeExceedTestCase ();

  void SendData (Ptr<Socket> socket, Ipv4Address dst);
  void DoSendData (Ptr<Socket> socket, Ipv4Address dst);
  void ReceivePkt (Ptr<Socket> socket);

public:
  virtual void DoRun (void);
  Ptr<Packet> m_receivedPacket;

};


IcmpTimeExceedTestCase::IcmpTimeExceedTestCase ()
  : TestCase ("ICMP:TimeExceedReply test case")
{

}


IcmpTimeExceedTestCase::~IcmpTimeExceedTestCase ()
{

}


void
IcmpTimeExceedTestCase::DoSendData (Ptr<Socket> socket, Ipv4Address dst)
{
  Ptr<Packet> p = Create<Packet> ();
  Icmpv4Echo echo;
  echo.SetSequenceNumber (1);
  echo.SetIdentifier (0);
  p->AddHeader (echo);

  Icmpv4Header header;
  header.SetType (Icmpv4Header::ICMPV4_ECHO);
  header.SetCode (0);
  p->AddHeader (header);

  Address realTo = InetSocketAddress (dst, 1234);

  NS_TEST_EXPECT_MSG_EQ (socket->SendTo (p, 0, realTo),
                         (int) p->GetSize (), " Unable to send ICMP Echo Packet");
}


void
IcmpTimeExceedTestCase::SendData (Ptr<Socket> socket, Ipv4Address dst)
{
  m_receivedPacket = Create<Packet> ();
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpTimeExceedTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}


void
IcmpTimeExceedTestCase::ReceivePkt (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (0xffffffff, 0, from);
  m_receivedPacket = p->Copy ();

  Ipv4Header ipv4;
  p->RemoveHeader (ipv4);
  NS_TEST_EXPECT_MSG_EQ (ipv4.GetProtocol (), 1,"The received packet is not an ICMP packet");

  NS_TEST_EXPECT_MSG_EQ (ipv4.GetSource (),Ipv4Address ("10.0.0.2"),
                         "ICMP Time Exceed Response should come from 10.0.0.2");

  Icmpv4Header icmp;
  p->RemoveHeader (icmp);

  NS_TEST_EXPECT_MSG_EQ (icmp.GetType (), Icmpv4Header::ICMPV4_TIME_EXCEEDED,
                         "The received packet is not a ICMPV4_TIME_EXCEEDED packet ");
}


void
IcmpTimeExceedTestCase::DoRun ()
{

  printf("Iniciando IcmpTimeExceedTestCase... \n");
  NodeContainer n, n0n1,n1n2;
  n.Create (3);
  n0n1.Add (n.Get (0));
  n0n1.Add (n.Get (1));
  n1n2.Add (n.Get (1));
  n1n2.Add (n.Get (2));

  Ptr<SimpleChannel> channel = CreateObject <SimpleChannel> ();
  Ptr<SimpleChannel> channel2 = CreateObject <SimpleChannel> ();

  SimpleNetDeviceHelper simpleHelper;
  simpleHelper.SetNetDevicePointToPointMode (true);

  SimpleNetDeviceHelper simpleHelper2;
  simpleHelper2.SetNetDevicePointToPointMode (true);

  NetDeviceContainer devices;
  devices = simpleHelper.Install (n0n1,channel);
  NetDeviceContainer devices2;
  devices2 = simpleHelper2.Install (n1n2,channel2);

  InternetStackHelper internet;
  internet.Install (n);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0","255.255.255.255");
  Ipv4InterfaceContainer i = address.Assign (devices);

  address.SetBase ("10.0.1.0","255.255.255.255");
  Ipv4InterfaceContainer i2 = address.Assign (devices2);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (n.Get (0), TypeId::LookupByName ("ns3::Ipv4RawSocketFactory"));
  socket->SetAttribute ("Protocol", UintegerValue (1)); // ICMP protocol
  socket->SetRecvCallback (MakeCallback (&IcmpTimeExceedTestCase::ReceivePkt, this));

  InetSocketAddress src = InetSocketAddress (Ipv4Address::GetAny (), 0);
  NS_TEST_EXPECT_MSG_EQ (socket->Bind (src),0," Socket Binding failed");


  // The ttl is not big enough , causing an ICMP Time Exceeded response
  socket->SetIpTtl (1);
  SendData (socket, i2.GetAddress (1,0));

  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 56, " Unexpected ICMP Time Exceed Response packet size");

  printf("Fim do IcmpTimeExceedTestCase... \n");
  Simulator::Destroy ();
}


/**
 * \ingroup icmp-test
 * \ingroup tests
 *
 * \brief ICMPV6  Echo Reply Test
 */
class IcmpV6EchoReplyTestCase : public TestCase
{
public:
  IcmpV6EchoReplyTestCase ();
  virtual ~IcmpV6EchoReplyTestCase ();

  void SendData (Ptr<Socket> socket, Ipv6Address dst);
  void DoSendData (Ptr<Socket> socket, Ipv6Address dst);
  void ReceivePkt (Ptr<Socket> socket);

public:
  virtual void DoRun (void);
  Ptr<Packet> m_receivedPacket;

};


IcmpV6EchoReplyTestCase::IcmpV6EchoReplyTestCase ()
  : TestCase ("ICMPV6:EchoReply test case")
{

}


IcmpV6EchoReplyTestCase::~IcmpV6EchoReplyTestCase ()
{

}


void
IcmpV6EchoReplyTestCase::DoSendData (Ptr<Socket> socket, Ipv6Address dst)
{
  Ptr<Packet> p = Create<Packet> ();
  Icmpv6Echo echo (1);
  echo.SetSeq (1);
  echo.SetId (0XB1ED);
  p->AddHeader (echo);

  Icmpv6Header header;
  header.SetType (Icmpv6Header::ICMPV6_ECHO_REQUEST);
  header.SetCode (0);
  p->AddHeader (header);

  Address realTo = Inet6SocketAddress (dst, 1234);

  NS_TEST_EXPECT_MSG_EQ (socket->SendTo (p, 0, realTo),
                         (int) p->GetSize (), " Unable to send ICMP Echo Packet");

}


void
IcmpV6EchoReplyTestCase::SendData (Ptr<Socket> socket, Ipv6Address dst)
{
  m_receivedPacket = Create<Packet> ();
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpV6EchoReplyTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpV6EchoReplyTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (from);
  m_receivedPacket = p->Copy ();

  if (Inet6SocketAddress::IsMatchingType (from))
    {
      Ipv6Header ipv6;
      p->RemoveHeader (ipv6);

      NS_TEST_EXPECT_MSG_EQ (ipv6.GetNextHeader (),
                             Ipv6Header::IPV6_ICMPV6,
                             "The received Packet is not an ICMPV6 packet");
      Icmpv6Header icmpv6;
      p->RemoveHeader (icmpv6);

      // Ignore the neighbor discovery (ICMPV6_ND) packets
      if (!(((int)icmpv6.GetType () >= 133) && ((int)icmpv6.GetType () <= 137)))
        {
          NS_TEST_EXPECT_MSG_EQ ((int) icmpv6.GetType (),
                                 Icmpv6Header::ICMPV6_ECHO_REPLY,
                                 "The received Packet is not a ICMPV6_ECHO_REPLY");
        }
    }
}


void
IcmpV6EchoReplyTestCase::DoRun ()
{

  printf("Iniciando IcmpV6EchoReplyTestCase... \n");
  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);

  // link the two nodes
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();
  txDev->SetAddress (Mac48Address ("00:00:00:00:00:01"));
  rxDev->SetAddress (Mac48Address ("00:00:00:00:00:02"));
  n.Get (0)->AddDevice (txDev);
  n.Get (1)->AddDevice (rxDev);
  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
  rxDev->SetChannel (channel1);
  txDev->SetChannel (channel1);
  NetDeviceContainer d;
  d.Add (txDev);
  d.Add (rxDev);

  Ipv6AddressHelper ipv6;

  ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i = ipv6.Assign (d);

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (n.Get (0), TypeId::LookupByName ("ns3::Ipv6RawSocketFactory"));
  socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
  socket->SetRecvCallback (MakeCallback (&IcmpV6EchoReplyTestCase::ReceivePkt, this));

  Inet6SocketAddress src = Inet6SocketAddress (Ipv6Address::GetAny (), 0);
  NS_TEST_EXPECT_MSG_EQ (socket->Bind (src),0," SocketV6 Binding failed");

  // Set a TTL big enough
  socket->SetIpTtl (1);

  SendData (socket, i.GetAddress (1,1));

  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 72, " Unexpected ICMPV6_ECHO_REPLY packet size");

  printf("Fim do IcmpV6EchoReplyTestCase... \n");
  Simulator::Destroy ();
}


/**
 * \ingroup icmp-test
 * \ingroup tests
 *
 * \brief ICMPV6  Time Exceed response test
 */
class IcmpV6TimeExceedTestCase : public TestCase
{
public:
  IcmpV6TimeExceedTestCase ();
  virtual ~IcmpV6TimeExceedTestCase ();

  void SendData (Ptr<Socket> socket, Ipv6Address dst);
  void DoSendData (Ptr<Socket> socket, Ipv6Address dst);
  void ReceivePkt (Ptr<Socket> socket);

public:
  virtual void DoRun (void);
  Ptr<Packet> m_receivedPacket;

};


IcmpV6TimeExceedTestCase::IcmpV6TimeExceedTestCase ()
  : TestCase ("ICMPV6:TimeExceed test case")
{

}


IcmpV6TimeExceedTestCase::~IcmpV6TimeExceedTestCase ()
{

}


void
IcmpV6TimeExceedTestCase::DoSendData (Ptr<Socket> socket, Ipv6Address dst)
{
  Ptr<Packet> p = Create<Packet> ();
  Icmpv6Echo echo (1);
  echo.SetSeq (1);
  echo.SetId (0XB1ED);
  p->AddHeader (echo);

  Icmpv6Header header;
  header.SetType (Icmpv6Header::ICMPV6_ECHO_REQUEST);
  header.SetCode (0);
  p->AddHeader (header);

  Address realTo = Inet6SocketAddress (dst, 1234);


  socket->SendTo (p, 0, realTo);

}


void
IcmpV6TimeExceedTestCase::SendData (Ptr<Socket> socket, Ipv6Address dst)
{
  m_receivedPacket = Create<Packet> ();
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpV6TimeExceedTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpV6TimeExceedTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (from);
  m_receivedPacket = p->Copy ();

  if (Inet6SocketAddress::IsMatchingType (from))
    {
      Ipv6Header ipv6;
      p->RemoveHeader (ipv6);


      NS_TEST_EXPECT_MSG_EQ (ipv6.GetNextHeader (),
                             Ipv6Header::IPV6_ICMPV6,
                             "The received Packet is not an ICMPV6 packet");

      Icmpv6Header icmpv6;
      p->RemoveHeader (icmpv6);

      // Ignore the neighbor discovery (ICMPV6_ND) packets
      if (!(((int)icmpv6.GetType () >= 133) && ((int)icmpv6.GetType () <= 137)))
        {
          NS_TEST_EXPECT_MSG_EQ ((int) icmpv6.GetType (),
                                 Icmpv6Header::ICMPV6_ERROR_TIME_EXCEEDED,
                                 "The received Packet is not a ICMPV6_ERROR_TIME_EXCEEDED");
        }
    }
}

void
IcmpV6TimeExceedTestCase::DoRun ()
{
  printf("Iniciando IcmpV6TimeExceedTestCase... \n");
  NodeContainer n, n0n1,n1n2;
  n.Create (3);
  n0n1.Add (n.Get (0));
  n0n1.Add (n.Get (1));
  n1n2.Add (n.Get (1));
  n1n2.Add (n.Get (2));

  Ptr<SimpleChannel> channel = CreateObject <SimpleChannel> ();
  Ptr<SimpleChannel> channel2 = CreateObject <SimpleChannel> ();

  SimpleNetDeviceHelper simpleHelper;
  simpleHelper.SetNetDevicePointToPointMode (true);

  SimpleNetDeviceHelper simpleHelper2;
  simpleHelper2.SetNetDevicePointToPointMode (true);

  NetDeviceContainer devices;
  devices = simpleHelper.Install (n0n1,channel);

  NetDeviceContainer devices2;
  devices2 = simpleHelper2.Install (n1n2,channel2);

  InternetStackHelper internet;
  internet.Install (n);

  Ipv6AddressHelper address;

  address.NewNetwork ();
  address.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));

  Ipv6InterfaceContainer interfaces = address.Assign (devices);
  interfaces.SetForwarding (1,true);
  interfaces.SetDefaultRouteInAllNodes (1);
  address.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer interfaces2 = address.Assign (devices2);

  interfaces2.SetForwarding (0,true);
  interfaces2.SetDefaultRouteInAllNodes (0);

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (n.Get (0), TypeId::LookupByName ("ns3::Ipv6RawSocketFactory"));
  socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
  socket->SetRecvCallback (MakeCallback (&IcmpV6TimeExceedTestCase::ReceivePkt, this));

  Inet6SocketAddress src = Inet6SocketAddress (Ipv6Address::GetAny (), 0);
  NS_TEST_EXPECT_MSG_EQ (socket->Bind (src),0," SocketV6 Binding failed");

  // In Ipv6 TTL is renamed hop limit in IPV6.
  // The hop limit is not big enough , causing an ICMPV6 Time Exceeded error
  socket->SetIpv6HopLimit (1);

  SendData (socket, interfaces2.GetAddress (1,1));

  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 72, " Unexpected ICMPV6_ECHO_REPLY packet size");


  printf("Fim do IcmpV6TimeExceedTestCase... \n");
  Simulator::Destroy ();
}

/**
 * \ingroup icmp-test
 * \ingroup tests
 *
 * \brief ICMP TestSuite
 */

class IcmpTestSuite : public TestSuite
{
public:
  IcmpTestSuite ();
};

IcmpTestSuite::IcmpTestSuite ()
  : TestSuite ("icmp", UNIT)
{
  AddTestCase (new IcmpEchoReplyTestCase, TestCase::QUICK);
  AddTestCase (new IcmpTimeExceedTestCase, TestCase::QUICK);
  AddTestCase (new IcmpV6EchoReplyTestCase, TestCase::QUICK);
  AddTestCase (new IcmpV6TimeExceedTestCase, TestCase::QUICK);
}

static IcmpTestSuite icmpTestSuite; //!< Static variable for test initialization

int main (int argc, char *argv[])
{
  IcmpEchoReplyTestCase icmpEchoReplyTest;
  icmpEchoReplyTest.DoRun();

  IcmpTimeExceedTestCase icmpTimeExceedTest;
  icmpTimeExceedTest.DoRun();

  IcmpV6EchoReplyTestCase icmpV6EchoReplyTest;
  icmpV6EchoReplyTest.DoRun();

  IcmpV6TimeExceedTestCase icmpV6TimeExceedTest;
  icmpV6TimeExceedTest.DoRun();
  
  printf("\n fim do main ");
  return 0;
}

