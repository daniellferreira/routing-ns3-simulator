
/* Network topology

                  net2
       / RouterA ------- RouterB \
      |         \       /         |
 net1 |     net7 \     /          | net3
      |           \   /           |
     /             \ /             \
HostT               X               HostR
     \             / \             /
      |           /   \           |
 net4 |     net8 /     \          | net6
      |         /       \         |  
       \ RouterC ------- RouterD /
                  net5

*/

#include <fstream>
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
  double simulationTime = 300.0; //seconds
  std::string transportProt = "Udp";

  // The below value configures the default behavior of global routing.
  // By default, it is disabled.  To respond to interface events, set to true
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  //Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));

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
  Ptr<Node> d = CreateObject<Node> ();
  Names::Add ("RouterD", d);
  
//redes: considerando que cada enlace é uma rede separada, sem meio compartilhado
  NodeContainer net1 (src, a); 
  NodeContainer net2 (a, b);   
  NodeContainer net3 (b, dst);
  NodeContainer net4 (src, c);
  NodeContainer net5 (c, d);
  NodeContainer net6 (d, dst);
  NodeContainer net7 (a, d);
  NodeContainer net8 (c, b);

  NodeContainer routers (a, b, c, d);
  NodeContainer nodes (src, dst);
  NS_LOG_WARN ("End create nodes.");

  NS_LOG_WARN ("Create channels.");
  // Enlaces usam CSMA e P2P para controle de acesso ao meio
  // É como se tivesse ligado os cabos entre as máquinas
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (5000000));
  p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer ndc1 = p2p.Install (net1);
  NetDeviceContainer ndc2 = p2p.Install (net2);
  NetDeviceContainer ndc3 = p2p.Install (net3);
  NetDeviceContainer ndc4 = p2p.Install (net4);
  NetDeviceContainer ndc5 = p2p.Install (net5);
  NetDeviceContainer ndc6 = p2p.Install (net6);
  NetDeviceContainer ndc7 = p2p.Install (net7);
  NetDeviceContainer ndc8 = p2p.Install (net8);

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

  ipv4.SetBase (Ipv4Address ("10.0.4.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic5 = ipv4.Assign (ndc5);

  ipv4.SetBase (Ipv4Address ("10.0.5.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic6 = ipv4.Assign (ndc6);

  ipv4.SetBase (Ipv4Address ("10.0.6.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic7 = ipv4.Assign (ndc7);

  ipv4.SetBase (Ipv4Address ("10.0.7.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic8 = ipv4.Assign (ndc8);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //NS_LOG_WARN ("DEBUG: Problem here.");

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
  apps.Stop (Seconds (simulationTime));

  //
  // Create a UdpEchoClient application to send UDP datagrams from node zero to
  // node one.
  //
  uint32_t packetSize = 1024;
  uint32_t maxPackets = simulationTime;
  Time interPacketInterval = Seconds (1.0);

  Ipv4Address serverAddress1 = iic3.GetAddress(1);
  UdpEchoClientHelper client1 (serverAddress1, port);
  client1.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  client1.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
  apps = client1.Install (src);
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (simulationTime));

  Ipv4Address serverAddress2 = iic6.GetAddress(1);
  UdpEchoClientHelper client2 (serverAddress2, port);
  client2.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client2.SetAttribute ("PacketSize", UintegerValue (packetSize));
  client2.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
  apps = client2.Install (src);
  apps.Start (Seconds (1.5));
  apps.Stop (Seconds (simulationTime));  

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("tp2-ospf.tr"));
  p2p.EnablePcapAll ("tp2-ospf", true);

  NS_LOG_WARN ("Configuring Animation.");

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers);

  AnimationInterface anim ("tp2-ospf.anim.xml");
  anim.SetConstantPosition(src, 10.0, 10.0); //for node src
  anim.SetConstantPosition(a, 20.0, 0.0); //for router a
  anim.SetConstantPosition(b, 30.0, 0.0); //for router b
  anim.SetConstantPosition(c, 20.0, 20.0); //for router c
  anim.SetConstantPosition(d, 30.0, 20.0); //for router d
  anim.SetConstantPosition(dst, 40.0, 10.0); //for node dst

  anim.UpdateNodeDescription(0, "Host_T");
  anim.UpdateNodeDescription(1, "Host_R");
  anim.UpdateNodeDescription(2, "Router_A");
  anim.UpdateNodeDescription(3, "Router_B");
  anim.UpdateNodeDescription(4, "Router_C");
  anim.UpdateNodeDescription(5, "Router_D");

  NS_LOG_WARN ("Run Simulation.");
  Ptr<Ipv4> ipv4B = b->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  uint32_t ipv4ifIndex1 = 1;
  Simulator::Schedule (Seconds (30.00), &Ipv4::SetDown, ipv4B, ipv4ifIndex1);
  Simulator::Schedule (Seconds (40.00), &Ipv4::SetUp, ipv4B, ipv4ifIndex1);

  Ptr<Ipv4> ipv4D = d->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  Simulator::Schedule (Seconds (70.00), &Ipv4::SetDown, ipv4D, ipv4ifIndex1);
  Simulator::Schedule (Seconds (90.00), &Ipv4::SetUp, ipv4D, ipv4ifIndex1);

  Simulator::Stop (Seconds(simulationTime)); // parar a simulação após 131 segundos
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_WARN ("Done.");
}