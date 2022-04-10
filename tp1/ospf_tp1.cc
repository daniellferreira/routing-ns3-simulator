// Network topology
//			     net1				        net2          		 net3				        net4
//  HostT ---------- RouterA ---------- RouterB ---------- RouterC ---------- HostR
//

// - Tracing of queues and packet receptions to file "dynamic-global-routing.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h" //the header file for animation
#include "ns3/mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DynamicGlobalRoutingExample");

int 
main (int argc, char *argv[])
{
  // Adicionado
  bool verbose = true;

  // The below value configures the default behavior of global routing.
  // By default, it is disabled.  To respond to interface events, set to true
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));

  // Allow the user to override any of the defaults and the above
  // Bind ()s at run-time, via command-line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.Parse (argc, argv);

   if (verbose)
  {
    LogComponentEnableAll (LogLevel (LOG_PREFIX_TIME | LOG_PREFIX_NODE));
    LogComponentEnable ("DynamicGlobalRoutingExample", LOG_LEVEL_WARN);
  }

  NS_LOG_WARN ("Create nodes.");

  // Node: elemento da rede, pode ser um host, roteador ou até mais alto nivel como aplicacao
  Ptr<Node> src = CreateObject<Node> ();
  Names::Add ("HostT", src);
  Ptr<Node> dst = CreateObject<Node> ();
  Names::Add ("HostR", dst);
  
  Ptr<Node> a = CreateObject<Node> ();
  Names::Add ("RouterA", a);
  Ptr<Node> b = CreateObject<Node> ();
  Names::Add ("RouterB", b);
  Ptr<Node> c = CreateObject<Node> ();
  Names::Add ("RouterC", c);
  
//redes: considerando que cada enlace é uma rede separada, sem meio compartilhado
  NodeContainer net1 (src, a);
  NodeContainer net2 (a, b);
  NodeContainer net3 (b, c);
  NodeContainer net4 (c, dst);
  NodeContainer routers (a, b, c);
  NodeContainer nodes (src, dst);

  NS_LOG_INFO ("Create channels.");
  // Enlaces usam CSMA para controle de acesso ao meio
  // É como se tivesse ligado os cabos entre as máquinas

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer ndc1 = csma.Install (net1);
  NetDeviceContainer ndc2 = csma.Install (net2);
  NetDeviceContainer ndc3 = csma.Install (net3);
  NetDeviceContainer ndc4 = csma.Install (net4);

  NS_LOG_INFO ("Create IPv4 and routing.");
  
  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false); //sem Ipv6
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false); //sem Ipv6
  internetNodes.Install (nodes);

  // Later, we add IP addresses.
  NS_LOG_WARN ("Assign IP Addresses.");

  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  ipv4.SetBase (Ipv4Address ("10.0.2.1"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc4);
  
  // Parei aqui

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  NS_LOG_WARN ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (i5i6.GetAddress (1), port));
  onoff.SetConstantRate (DataRate ("2kbps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (50));

  ApplicationContainer apps = onoff.Install (c.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  // Create a second OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  OnOffHelper onoff2 ("ns3::UdpSocketFactory",
                      InetSocketAddress (i1i6.GetAddress (1), port));
  onoff2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff2.SetAttribute ("DataRate", StringValue ("2kbps"));
  onoff2.SetAttribute ("PacketSize", UintegerValue (50));

  ApplicationContainer apps2 = onoff2.Install (c.Get (1));
  apps2.Start (Seconds (11.0));
  apps2.Stop (Seconds (16.0));

  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  apps = sink.Install (c.Get (6));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  PacketSinkHelper sink2 ("ns3::UdpSocketFactory",
                          Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  apps2 = sink2.Install (c.Get (6));
  apps2.Start (Seconds (11.0));
  apps2.Stop (Seconds (16.0));


  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("ospf_tp1.tr");
  p2p.EnableAsciiAll (stream);
  csma.EnableAsciiAll (stream);
  internet.EnableAsciiIpv4All (stream);

  p2p.EnablePcapAll ("ospf_tp1");
  csma.EnablePcapAll ("ospf_tp1", false);
 
  Ptr<Node> n1 = c.Get (1);
  Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  uint32_t ipv4ifIndex1 = 2;

  Simulator::Schedule (Seconds (2),&Ipv4::SetDown,ipv41, ipv4ifIndex1);
  Simulator::Schedule (Seconds (4),&Ipv4::SetUp,ipv41, ipv4ifIndex1);

  Ptr<Node> n6 = c.Get (6);
  Ptr<Ipv4> ipv46 = n6->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  uint32_t ipv4ifIndex6 = 2;
  Simulator::Schedule (Seconds (6),&Ipv4::SetDown,ipv46, ipv4ifIndex6);
  Simulator::Schedule (Seconds (8),&Ipv4::SetUp,ipv46, ipv4ifIndex6);

  Simulator::Schedule (Seconds (12),&Ipv4::SetDown,ipv41, ipv4ifIndex1);
  Simulator::Schedule (Seconds (14),&Ipv4::SetUp,ipv41, ipv4ifIndex1);

  // Trace routing tables 
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("ospf_tp1.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (12), routingStream);

  // Adicionado
  /* NS_LOG_WARN ("Configuring Animation.");

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers); */

  // Adicionado
  /* AnimationInterface anim ("exemplo-rip.anim.xml");
  anim.SetConstantPosition(src, 10.0, 10.0); //for node src
  anim.SetConstantPosition(a, 20.0, 10.0); //for router a
  anim.SetConstantPosition(b, 30.0, 10.0); //for router b
  anim.SetConstantPosition(dst, 40.0, 10.0); //for node dst */
  // Adicionado
  /* anim.UpdateNodeDescription(0, "src");
  anim.UpdateNodeDescription(1, "dst");
  anim.UpdateNodeDescription(2, "a");
  anim.UpdateNodeDescription(3, "b"); */

  NS_LOG_WARN ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_WARN ("Done.");
}
