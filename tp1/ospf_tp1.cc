// Network topology

//			     net1				        net2          		 net3				        net4
//  HostT ---------- RouterA ---------- RouterB ---------- RouterC ---------- HostR

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

int main (int argc, char *argv[])
{
  // Adicionado
  bool verbose = true;
  double simulationTime = 131.0; //seconds
  std::string transportProt = "Udp";

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
  NS_LOG_WARN ("End create nodes.");

  NS_LOG_WARN ("Create channels.");
  // Enlaces usam CSMA para controle de acesso ao meio
  // É como se tivesse ligado os cabos entre as máquinas
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer ndc1 = csma.Install (net1);
  NetDeviceContainer ndc2 = csma.Install (net2);
  NetDeviceContainer ndc3 = csma.Install (net3);
  NetDeviceContainer ndc4 = csma.Install (net4);

  NS_LOG_WARN ("Create IPv4 and routing.");

  // Na implementacao do Global routing nao tem essa parte do RoutingHelper.
  
  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false); //sem Ipv6
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false); //sem Ipv6
  internetNodes.Install (nodes);

  // Later, we add IP addresses.
  NS_LOG_WARN ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;

  //dando endereços de rede para os enlaces
  //como se tivesse usado DHCP para os enlaces entre os nodos

  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  ipv4.SetBase (Ipv4Address ("10.0.3.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // Creio que isso fico no lugar do Ptr<Ipv4StaticRouting> staticRouting; 
  // no caso do rip

  NS_LOG_WARN ("Create Applications.");
  //
  // Create a UdpEchoServer application on node one.
  //
  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (dst);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (simulationTime - 10.0));

  //
  // Create a UdpEchoClient application to send UDP datagrams from node zero to
  // node one.
  //
  uint32_t packetSize = 1024;
  Time interPacketInterval = Seconds (1.0);

  UdpEchoClientHelper client (iic4.GetAddress(1), port);
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (src);
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (simulationTime - 20.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("tp1-ospf.tr"));
  csma.EnablePcapAll ("tp1-ospf", true);

  NS_LOG_WARN ("Configuring Animation.");

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers);

  AnimationInterface anim ("tp1-ospf.anim.xml");
  anim.SetConstantPosition(src, 10.0, 10.0); //for node src
  anim.SetConstantPosition(a, 20.0, 10.0); //for router a
  anim.SetConstantPosition(b, 30.0, 10.0); //for router b
  anim.SetConstantPosition(c, 40.0, 10.0); //for router b
  anim.SetConstantPosition(dst, 50.0, 10.0); //for node dst

  anim.UpdateNodeDescription(0, "Host_T");
  anim.UpdateNodeDescription(1, "Host_R");
  anim.UpdateNodeDescription(2, "Router_A");
  anim.UpdateNodeDescription(3, "Router_B");
  anim.UpdateNodeDescription(4, "Router_C");

  // Trace routing tables 
  // Nao sei dizer se fica, ele [e parecido com o  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("ospf_tp1.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (12), routingStream);

  NS_LOG_WARN ("Run Simulation.");
  Simulator::Stop (Seconds(simulationTime)); // parar a simulação após 131 segundos
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_WARN ("Done.");
}