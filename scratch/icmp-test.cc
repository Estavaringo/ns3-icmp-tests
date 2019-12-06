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
  echo.SetIdentifier (5);
  p->AddHeader (echo);


  Icmpv4Header header;
  header.SetType (Icmpv4Header::ICMPV4_ECHO);
  header.SetCode (0);
  p->AddHeader (header);

  Address realTo = InetSocketAddress (dst, 1234);

  if(socket->SendTo (p, 0, realTo) != (int) p->GetSize ()){
    printf("Falha ao enviar Pacote ICMP Echo request\n\n");
    return;
  }


  printf("Pacote Enviado: \n");
  p->Print(std::cout);
  printf("\n\n");

}


void
IcmpEchoReplyTestCase::SendData (Ptr<Socket> socket, Ipv4Address dst)
{
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpEchoReplyTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpEchoReplyTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (0xffffffff, 0, from);

  printf("Pacote Recebido: \n");
  p->Print(std::cout);
  printf("\n\n");

  Ipv4Header ipv4;
  p->RemoveHeader (ipv4);

  if(ipv4.GetProtocol () != 1){
    printf("O pacote recebido não é um pacote ICMP\n\n");
  }

  Icmpv4Header icmp;
  p->RemoveHeader (icmp);

  if(icmp.GetType () != Icmpv4Header::ICMPV4_ECHO_REPLY){
    printf("O pacote recebido não é um pacote ICMP Echo Reply\n\n");
  }
}


void
IcmpEchoReplyTestCase::DoRun ()
{
  printf("Iniciando IcmpEchoReplyTestCase... \n\n");
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
  
  if(socket->Bind (src) != 0){
    printf("Falha ao efetuar socket bind\n\n");
    printf("Finalizando IcmpEchoReplyTestCase\n\n");
    return;
  }

  socket->SetIpTtl (1);
  SendData (socket, i.GetAddress (1,0));


  printf("Finalizando IcmpEchoReplyTestCase!\n\n");
  Simulator::Destroy ();

  printf("\n\n");
}


/**
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


  if(socket->SendTo (p, 0, realTo) != (int) p->GetSize ()){
    printf("Falha ao enviar Pacote ICMP Echo request\n\n");
    return;
  }

  printf("Pacote Enviado:\n");
  p->Print(std::cout);
  printf("\n\n");

}


void
IcmpTimeExceedTestCase::SendData (Ptr<Socket> socket, Ipv4Address dst)
{
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpTimeExceedTestCase::DoSendData, this, socket, dst);

  Simulator::Run ();
}


void
IcmpTimeExceedTestCase::ReceivePkt (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (0xffffffff, 0, from);
  
  printf("Pacote Recebido: \n");
  p->Print(std::cout);
  printf("\n \n");

  Ipv4Header ipv4;
  p->RemoveHeader (ipv4);
  if(ipv4.GetProtocol () != 1){
    printf("O pacote recebido não é um pacote ICMP\n\n");
  }
  if(ipv4.GetSource () != Ipv4Address ("10.0.0.2")){
    printf("A resposta de Time Exceeded deveria vir do nó 10.0.0.2\n\n");
  }

  Icmpv4Header icmp;
  p->RemoveHeader (icmp);
  if(icmp.GetType () != Icmpv4Header::ICMPV4_TIME_EXCEEDED){
    printf("O pacote recebido não é um pacote ICMP Time Exceeded\n\n");
  }
}


void
IcmpTimeExceedTestCase::DoRun ()
{
  printf("Iniciando IcmpTimeExceedTestCase... \n\n");
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

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (n.Get (0), TypeId::LookupByName ("ns3::Ipv4RawSocketFactory"));
  socket->SetAttribute ("Protocol", UintegerValue (1)); // ICMP protocol
  socket->SetRecvCallback (MakeCallback (&IcmpTimeExceedTestCase::ReceivePkt, this));


  InetSocketAddress src = InetSocketAddress (Ipv4Address::GetAny (), 0);
  if(socket->Bind (src) != 0){
    printf("Falha ao efetuar socket bind\n\n");
    printf("Finalizando IcmpTimeExceedTestCase!\n\n");
    return;
  }


  // O TTL deve ser pequeno o suficiente para causa uma resposta ICMP Time Exceeded
  socket->SetIpTtl (1);
  SendData (socket, i2.GetAddress (1,0));

  printf("Finalizando IcmpTimeExceedTestCase!\n");
  Simulator::Destroy ();
  printf("\n\n");
}


/**
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

  socket->SendTo (p, 0, realTo);

  printf("Pacote Enviado:\n");
  p->Print(std::cout);
  printf("\n\n");

}


void
IcmpV6EchoReplyTestCase::SendData (Ptr<Socket> socket, Ipv6Address dst)
{
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpV6EchoReplyTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpV6EchoReplyTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (from);

  printf("Pacote recebido:\n");
  p->Print(std::cout);
  printf("\n\n");

  if (Inet6SocketAddress::IsMatchingType (from))
    {
      Ipv6Header ipv6;
      p->RemoveHeader (ipv6);
      if(ipv6.GetNextHeader () != Ipv6Header::IPV6_ICMPV6){
        printf("O pacote recebido não é um pacote ICMPV6\n\n");
      }

      Icmpv6Header icmpv6;
      p->RemoveHeader (icmpv6);
      // Ignora os pacotes de neighbor discovery
      //Type 133 - Router Solicitation
      //Type 134 - Router Advertisement
      //Type 135 - Neighbor Solicitation
      //Type 136 - Neighbor Advertisement
      //Type 137 - Redirect Message
      if (!(((int)icmpv6.GetType () >= 133) && ((int)icmpv6.GetType () <= 137)))
        {

          if((int) icmpv6.GetType () != Icmpv6Header::ICMPV6_ECHO_REPLY){
            printf("O pacote recebido não é um pacote ICMPV6 Echo Reply\n\n");
          }
        }

    }
}


void
IcmpV6EchoReplyTestCase::DoRun ()
{

  printf("Iniciando IcmpV6EchoReplyTestCase: \n\n");
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
  
  if(socket->Bind (src) != 0){
    printf("Falha ao efetuar socket bind\n\n");
    printf("Finalizando IcmpV6EchoReplyTestCase\n\n");
    return;
  }

  socket->SetIpTtl (1);

  SendData (socket, i.GetAddress (1,1));

  printf("Finalizando IcmpV6EchoReplyTestCase!\n\n");
  Simulator::Destroy ();
}


/**
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

  printf("Pacote Enviado: \n");
  p->Print(std::cout);
  printf("\n\n");

}


void
IcmpV6TimeExceedTestCase::SendData (Ptr<Socket> socket, Ipv6Address dst)
{
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpV6TimeExceedTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpV6TimeExceedTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (from);

  printf("Pacote recebido:\n");
  p->Print(std::cout);
  printf("\n\n");

  if (Inet6SocketAddress::IsMatchingType (from))
    {

      Ipv6Header ipv6;
      p->RemoveHeader (ipv6);
      if(ipv6.GetNextHeader () != Ipv6Header::IPV6_ICMPV6){
        printf("O pacote recebido não é um pacote ICMPV6\n\n");
      }
  

      Icmpv6Header icmpv6;
      p->RemoveHeader (icmpv6);

      // Ignora os pacotes de neighbor discovery
      //Type 133 - Router Solicitation
      //Type 134 - Router Advertisement
      //Type 135 - Neighbor Solicitation
      //Type 136 - Neighbor Advertisement
      //Type 137 - Redirect Message
      if (!(((int)icmpv6.GetType () >= 133) && ((int)icmpv6.GetType () <= 137)))
        {
          if((int) icmpv6.GetType () != Icmpv6Header::ICMPV6_ERROR_TIME_EXCEEDED){
            printf("O pacote recebido não é um pacote ICMPV6 Time Exceeded\n\n");
          }
        }
    }
}

void
IcmpV6TimeExceedTestCase::DoRun ()
{
	printf("Iniciando IcmpV6TimeExceedTestCase: \n\n");
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
  if(socket->Bind (src) != 0){
    printf("Falha ao efetuar socket bind\n\n");
    printf("Finalizando IcmpV6TimeExceedTestCase\n\n");
    return;
  }

  // O TTL (Hop Limit) deve ser pequeno o suficiente para causa uma resposta ICMPV6 Time Exceeded
  socket->SetIpv6HopLimit (1);

  SendData (socket, interfaces2.GetAddress (1,1));

  printf("Finalizando IcmpV6TimeExceedTestCase!\n\n");
  Simulator::Destroy ();
}

/**
 * \brief ICMP Destination Unreachable Reply Test
 */
class IcmpDestinationUnreachableTestCase : public TestCase
{
public:
  IcmpDestinationUnreachableTestCase ();
  virtual ~IcmpDestinationUnreachableTestCase ();

  void SendData (Ptr<Socket> socket, Ipv4Address dst);
  void DoSendData (Ptr<Socket> socket, Ipv4Address dst);
  void ReceivePkt (Ptr<Socket> socket);

public:
  virtual void DoRun (void);

};


IcmpDestinationUnreachableTestCase::IcmpDestinationUnreachableTestCase ()
  : TestCase ("ICMP:DestinationUnreachableReply test case")
{

}


IcmpDestinationUnreachableTestCase::~IcmpDestinationUnreachableTestCase ()
{

}


void
IcmpDestinationUnreachableTestCase::DoSendData (Ptr<Socket> socket, Ipv4Address dst)
{
  Ptr<Packet> p = Create<Packet> ();
  Icmpv4Echo echo;
  echo.SetSequenceNumber (1);
  echo.SetIdentifier (0);
  p->AddHeader (echo);


  Icmpv4Header header;
  header.SetType (Icmpv4Header::ICMPV4_DEST_UNREACH);
  header.SetCode (0);
  p->AddHeader (header);

  Address realTo = InetSocketAddress (dst, 1234);

  if(socket->SendTo (p, 0, realTo) != (int) p->GetSize ()){
    printf("Falha ao enviar Pacote ICMP Echo request\n\n");
    return;
  }

  printf("Pacote Enviado: \n");
  p->Print(std::cout);
  printf("\n \n");

}


void
IcmpDestinationUnreachableTestCase::SendData (Ptr<Socket> socket, Ipv4Address dst)
{
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpDestinationUnreachableTestCase::DoSendData, this, socket, dst);

  Simulator::Run ();
}


void
IcmpDestinationUnreachableTestCase::ReceivePkt (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (0xffffffff, 0, from);

  printf("Pacote Recebido: \n");
  p->Print(std::cout);
  printf("\n\n");

  Ipv4Header ipv4;
  p->RemoveHeader (ipv4);
  if(ipv4.GetProtocol () != 1){
    printf("O pacote recebido não é um pacote ICMP\n\n");
  }

  Icmpv4Header icmp;
  p->RemoveHeader (icmp);
  if(icmp.GetType () != Icmpv4Header::ICMPV4_DEST_UNREACH){
    printf("O pacote recebido não é um pacote ICMP Destination Unreachable\n\n");
  }
}


void
IcmpDestinationUnreachableTestCase::DoRun ()
{
  printf("Iniciando IcmpDestinationUnreachableTestCase... \n\n");
  NodeContainer n, n0n1,n1n2;
  n.Create (4);
  n0n1.Add (n.Get (0));
  n0n1.Add (n.Get (1));
  n1n2.Add (n.Get (2));
  n1n2.Add (n.Get (3));

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

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (n.Get (0), TypeId::LookupByName ("ns3::Ipv4RawSocketFactory"));
  socket->SetAttribute ("Protocol", UintegerValue (1)); // ICMP protocol
  socket->SetRecvCallback (MakeCallback (&IcmpDestinationUnreachableTestCase::ReceivePkt, this));


  InetSocketAddress src = InetSocketAddress (Ipv4Address::GetAny (), 0);
  if(socket->Bind (src) != 0){
    printf("Falha ao efetuar socket bind\n\n");
    printf("Finalizando IcmpDestinationUnreachableTestCase\n\n");
    return;
  }

  socket->SetIpTtl (5);
  SendData (socket, Ipv4Address("10.0.1.5"));

  printf("Finalizando IcmpDestinationUnreachableTestCase!\n");
  Simulator::Destroy ();
  printf("\n\n");
}


/**
 * \brief ICMPV6  Destination Unreachable response test
 */
class IcmpV6DestinationUnreachableTestCase : public TestCase
{
public:
  IcmpV6DestinationUnreachableTestCase ();
  virtual ~IcmpV6DestinationUnreachableTestCase ();

  void SendData (Ptr<Socket> socket, Ipv6Address dst);
  void DoSendData (Ptr<Socket> socket, Ipv6Address dst);
  void ReceivePkt (Ptr<Socket> socket);

public:
  virtual void DoRun (void);

};


IcmpV6DestinationUnreachableTestCase::IcmpV6DestinationUnreachableTestCase ()
  : TestCase ("ICMPV6:DestinationUnreachable test case")
{

}


IcmpV6DestinationUnreachableTestCase::~IcmpV6DestinationUnreachableTestCase ()
{

}


void
IcmpV6DestinationUnreachableTestCase::DoSendData (Ptr<Socket> socket, Ipv6Address dst)
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

  printf("Pacote Enviado:\n");
  p->Print(std::cout);
  printf("\n\n");

}


void
IcmpV6DestinationUnreachableTestCase::SendData (Ptr<Socket> socket, Ipv6Address dst)
{
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &IcmpV6DestinationUnreachableTestCase::DoSendData, this, socket, dst);
  Simulator::Run ();
}

void
IcmpV6DestinationUnreachableTestCase::ReceivePkt (Ptr <Socket> socket)
{
  Address from;
  Ptr<Packet> p = socket->RecvFrom (from);

  printf("Pacote recebido:\n");
  p->Print(std::cout);
  printf("\n\n");

  if (Inet6SocketAddress::IsMatchingType (from))
    {


      Ipv6Header ipv6;
      p->RemoveHeader (ipv6);

      if(ipv6.GetNextHeader () != Ipv6Header::IPV6_ICMPV6){
        printf("O pacote recebido não é um pacote ICMPV6\n\n");
      }

      Icmpv6Header icmpv6;
      p->RemoveHeader (icmpv6);

      // Ignora os pacotes de neighbor discovery
      //Type 133 - Router Solicitation
      //Type 134 - Router Advertisement
      //Type 135 - Neighbor Solicitation
      //Type 136 - Neighbor Advertisement
      //Type 137 - Redirect Message
      if (!(((int)icmpv6.GetType () >= 133) && ((int)icmpv6.GetType () <= 137)))
        {  
          if((int) icmpv6.GetType () != Icmpv6Header::ICMPV6_ERROR_DESTINATION_UNREACHABLE){
            printf("O pacote recebido não é um pacote ICMPV6 Destination Unreachable\n\n");
          }
        }
    }
}

void
IcmpV6DestinationUnreachableTestCase::DoRun ()
{
	printf("Iniciando IcmpV6DestinationUnreachableTestCase: \n\n");
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
  socket->SetRecvCallback (MakeCallback (&IcmpV6DestinationUnreachableTestCase::ReceivePkt, this));

  Inet6SocketAddress src = Inet6SocketAddress (Ipv6Address::GetAny (), 0);
  if(socket->Bind (src) != 0){
   printf("Falha ao efetuar socket bind\n\n");
   return;
  }

  socket->SetIpv6HopLimit (64);

  SendData (socket, Ipv6Address("2001:2::200:ff:fe00:5"));

  printf("Finalizando IcmpV6DestinationUnreachableTestCase!\n\n");

  Simulator::Destroy ();
}

int main (int argc, char *argv[])
{
  printf("\n\t Início das simulações\n\n");

	Packet::EnablePrinting();

  IcmpEchoReplyTestCase icmp;
  icmp.DoRun();

  IcmpV6EchoReplyTestCase icmpv6;
  icmpv6.DoRun();

  IcmpTimeExceedTestCase timeExceed;
  timeExceed.DoRun();

  IcmpV6TimeExceedTestCase timeExceedV6;
  timeExceedV6.DoRun();

  IcmpDestinationUnreachableTestCase destinationUnreachable;
  destinationUnreachable.DoRun();

  IcmpV6DestinationUnreachableTestCase destinationUnreachableV6;
  destinationUnreachableV6.DoRun();

  printf("\n\t Fim das simulações\n");
  return 0;
}
