#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/netanim-module.h" //the header file for animation
#include "ns3/mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RipSimpleRouting");

int main (int argc, char **argv)
{
  bool verbose = false;
  bool printRoutingTables = false;
  bool showPings = false;
  std::string SplitHorizon ("PoisonReverse");

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.AddValue ("printRountingTables", "Print routing tables at 30, 60 and 90 seconds", printRoutingTables);
  cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
  cmd.AddValue ("splitHorizonStrategy", "Split Horizon strategy to use (NoSplitHorizon, SplitHorizon, PoisonReverse)", SplitHorizon);
  cmd.Parse (argc, argv);

  if (verbose)
  {
    LogComponentEnableAll (LogLevel (LOG_PREFIX_TIME | LOG_PREFIX_NODE));
    LogComponentEnable ("RipSimpleRouting", LOG_LEVEL_INFO);
    LogComponentEnable ("Rip", LOG_LEVEL_ALL);
    LogComponentEnable ("Ipv4Interface", LOG_LEVEL_ALL);
    LogComponentEnable ("Icmpv4L4Protocol", LOG_LEVEL_ALL);
    LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
    LogComponentEnable ("ArpCache", LOG_LEVEL_ALL);
    LogComponentEnable ("V4Ping", LOG_LEVEL_ALL);
  }

  if (SplitHorizon == "NoSplitHorizon")
  {
    Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::NO_SPLIT_HORIZON));
  }
  else if (SplitHorizon == "SplitHorizon")
  {
    Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::SPLIT_HORIZON));
  }
  else
  {
    Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::POISON_REVERSE));
  }

  NS_LOG_INFO ("Start create nodes.");
  // Node: elemento da rede, pode ser um host, roteador ou até mais alto nivel como aplicacao
  Ptr<Node> src = CreateObject<Node> ();
  Names::Add ("SrcNode", src);
  Ptr<Node> dst = CreateObject<Node> ();
  Names::Add ("DstNode", dst);
  
  Ptr<Node> a = CreateObject<Node> ();
  Names::Add ("RouterA", a);
  Ptr<Node> b = CreateObject<Node> ();
  Names::Add ("RouterB", b);

  //redes: considerando que cada enlace é uma rede separada, sem meio compartilhado
  NodeContainer net1 (src, a);
  NodeContainer net2 (a, b);
  NodeContainer net3 (b, dst);
  NodeContainer routers (a, b);
  NodeContainer nodes (src, dst);
  NS_LOG_INFO ("End create nodes.");
  
  NS_LOG_INFO ("Create channels.");
  // Enlaces usam CSMA para controle de acesso ao meio
  // É como se tivesse ligado os cabos entre as máquinas
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer ndc1 = csma.Install (net1);
  NetDeviceContainer ndc2 = csma.Install (net2);
  NetDeviceContainer ndc3 = csma.Install (net3);

  NS_LOG_INFO ("Create IPv4 and routing");
  RipHelper ripRouting;
  
  // Exclui as interfaces entre hosts e roteadores, pois queremos que o RIP aconteça somente entre os roteadores
  ripRouting.ExcludeInterface (a, 1); // exclui do RIP a interface entre src e a 
  ripRouting.ExcludeInterface (b, 2); // exclui do RIP a interface entre b e dst

  Ipv4ListRoutingHelper listRH;
  listRH.Add (ripRouting, 0);

  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false); //sem Ipv6
  internet.SetRoutingHelper (listRH); //qual objeto de roteamento (instala o protocolo de roteamento)
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false); //sem Ipv6
  internetNodes.Install (nodes);

  NS_LOG_INFO ("Assign IPv4 Addresses.");
  Ipv4AddressHelper ipv4;

  //dando endereços de rede para os enlaces
  //como se tivesse usado DHCP para os enlaces entre os nodos
  
  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  // configuração da rota padrão para os hosts
  Ptr<Ipv4StaticRouting> staticRouting;
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (src->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.0.2", 1 ); //src (10.0.0.1) ->  a (10.0.0.2)
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (dst->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.2.1", 1 ); //dst (10.0.2.2) -> b (10.0.2.1)

  if (printRoutingTables)
  {
    RipHelper routingHelper;

    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

    routingHelper.PrintRoutingTableAt (Seconds (30.0), a, routingStream);
    routingHelper.PrintRoutingTableAt (Seconds (30.0), b, routingStream);

    routingHelper.PrintRoutingTableAt (Seconds (60.0), a, routingStream);
    routingHelper.PrintRoutingTableAt (Seconds (60.0), b, routingStream);

    routingHelper.PrintRoutingTableAt (Seconds (90.0), a, routingStream);
    routingHelper.PrintRoutingTableAt (Seconds (90.0), b, routingStream);
  }

  NS_LOG_INFO ("Create Applications.");
  uint32_t packetSize = 1024; //bytes
  Time interPacketInterval = Seconds (1.0);
  V4PingHelper ping ("10.0.2.2"); //destino

  ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping.SetAttribute ("Size", UintegerValue (packetSize));
  if (showPings)
  {
    ping.SetAttribute ("Verbose", BooleanValue (true));
  }
  ApplicationContainer apps = ping.Install (src);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (110.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("exemplo-rip.tr"));
  csma.EnablePcapAll ("exemplo-rip", true);

  NS_LOG_INFO ("Configuring Animation.");

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  mobility.Install (routers);

  AnimationInterface anim ("exemplo-rip.anim.xml");
  anim.SetConstantPosition(src, 10.0, 10.0); //for node src
  anim.SetConstantPosition(a, 20.0, 10.0); //for router a
  anim.SetConstantPosition(b, 30.0, 10.0); //for router b
  anim.SetConstantPosition(dst, 40.0, 10.0); //for node dst

  anim.UpdateNodeDescription(0, "src");
  anim.UpdateNodeDescription(1, "dst");
  anim.UpdateNodeDescription(2, "a");
  anim.UpdateNodeDescription(3, "b");

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(131.0)); // parar a simulação após 131 segundos
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}