#!/usr/bin/python

"""
Leaf-Spine Topology for Mininet - Assignment 14

This creates a scalable leaf-spine (Clos) network topology.

Topology Structure:
- Spine switches: Core layer switches (fully meshed with leaf switches)
- Leaf switches: Access layer switches (connected to all spine switches)
- Hosts: Connected to leaf switches

Example with 2 spine, 4 leaf, 2 hosts per leaf:
    
         [Spine1]────────[Spine2]
         /  |  \  \    /  /  |  \
        /   |   \  \  /  /   |   \
    [L1]  [L2]  [L3]  [L4]
     ||    ||    ||    ||
    h1h2  h3h4  h5h6  h7h8
"""

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import RemoteController, OVSSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import TCLink

class LeafSpineTopo(Topo):
    """Leaf-Spine topology with configurable parameters"""
    
    def __init__(self, num_spine=2, num_leaf=4, hosts_per_leaf=2, **opts):
        """
        num_spine: Number of spine switches
        num_leaf: Number of leaf switches
        hosts_per_leaf: Number of hosts connected to each leaf
        """
        Topo.__init__(self, **opts)
        
        self.num_spine = num_spine
        self.num_leaf = num_leaf
        self.hosts_per_leaf = hosts_per_leaf
        
        # Create spine switches
        spine_switches = []
        info('*** Creating spine switches\n')
        for s in range(num_spine):
            spine_name = 'spine%d' % (s + 1)
            spine_switch = self.addSwitch(spine_name, dpid='%016x' % (s + 1))
            spine_switches.append(spine_switch)
            info('  Added %s\n' % spine_name)
        
        # Create leaf switches and connect to all spines
        leaf_switches = []
        info('*** Creating leaf switches and connecting to spines\n')
        for l in range(num_leaf):
            leaf_name = 'leaf%d' % (l + 1)
            leaf_switch = self.addSwitch(leaf_name, dpid='%016x' % (100 + l + 1))
            leaf_switches.append(leaf_switch)
            info('  Added %s\n' % leaf_name)
            
            # Connect this leaf to all spine switches (full mesh)
            for spine in spine_switches:
                self.addLink(leaf_switch, spine)
                info('    Connected %s <-> %s\n' % (leaf_name, spine))
        
        # Create hosts and connect to leaf switches
        info('*** Creating hosts and connecting to leaf switches\n')
        host_num = 1
        for leaf_idx, leaf in enumerate(leaf_switches):
            for h in range(hosts_per_leaf):
                host_name = 'h%d' % host_num
                host = self.addHost(host_name, ip='10.0.0.%d/24' % host_num)
                self.addLink(host, leaf)
                info('  Added %s connected to %s\n' % (host_name, leaf))
                host_num += 1
        
        info('*** Topology creation complete\n')
        info('  Total Spine Switches: %d\n' % num_spine)
        info('  Total Leaf Switches: %d\n' % num_leaf)
        info('  Total Hosts: %d\n' % (num_leaf * hosts_per_leaf))
        info('  Total Links: %d\n' % (num_spine * num_leaf + num_leaf * hosts_per_leaf))


def run_topology(num_spine=2, num_leaf=4, hosts_per_leaf=2):
    """Create and run the leaf-spine topology"""
    
    info('╔═══════════════════════════════════════════════════════════╗\n')
    info('║     Leaf-Spine Topology - Assignment 14                  ║\n')
    info('╠═══════════════════════════════════════════════════════════╣\n')
    info('║  Configuration:                                           ║\n')
    info('║    Spine Switches: %-2d                                   ║\n' % num_spine)
    info('║    Leaf Switches:  %-2d                                   ║\n' % num_leaf)
    info('║    Hosts per Leaf: %-2d                                   ║\n' % hosts_per_leaf)
    info('╚═══════════════════════════════════════════════════════════╝\n\n')
    
    # Create topology
    topo = LeafSpineTopo(num_spine=num_spine, 
                         num_leaf=num_leaf, 
                         hosts_per_leaf=hosts_per_leaf)
    
    # Create network
    net = Mininet(topo=topo, switch=OVSSwitch, link=TCLink, autoSetMacs=True)
    
    # Start network
    info('*** Starting network\n')
    net.start()
    
    # Print network information
    info('\n*** Network Information:\n')
    info('Hosts: %s\n' % [h.name for h in net.hosts])
    info('Switches: %s\n' % [s.name for s in net.switches])
    
    # Connectivity test
    info('\n*** Testing connectivity (pingAll)\n')
    net.pingAll()
    
    # Display topology diagram
    print_topology_diagram(num_spine, num_leaf, hosts_per_leaf)
    
    # Open CLI
    info('\n*** Running CLI (type "help" for commands)\n')
    info('*** Useful commands:\n')
    info('    pingall          - Test connectivity between all hosts\n')
    info('    iperf h1 h2      - Test bandwidth between h1 and h2\n')
    info('    xterm h1 h2      - Open terminals for h1 and h2\n')
    info('    dump             - Display network information\n')
    info('    net              - Display network links\n\n')
    
    CLI(net)
    
    # Stop network
    info('*** Stopping network\n')
    net.stop()


def print_topology_diagram(num_spine, num_leaf, hosts_per_leaf):
    """Print ASCII diagram of the topology"""
    print('\n╔═══════════════════════════════════════════════════════════╗')
    print('║                  TOPOLOGY DIAGRAM                         ║')
    print('╚═══════════════════════════════════════════════════════════╝')
    print('\nSpine Layer (Core):')
    print('  ', end='')
    for s in range(num_spine):
        print('[Spine%d]' % (s+1), end='  ')
    print('\n     ', end='')
    for s in range(num_spine):
        print('|  |', end='     ')
    print('\n     ', end='')
    for s in range(num_spine):
        print('╲  ╱', end='     ')
    print('\n')
    
    print('Leaf Layer (Access):')
    print('  ', end='')
    for l in range(num_leaf):
        print('[Leaf%d]' % (l+1), end='  ')
    print('\n  ', end='')
    for l in range(num_leaf):
        print('  ||  ', end='  ')
    print('\n')
    
    print('Host Layer:')
    host_num = 1
    print('  ', end='')
    for l in range(num_leaf):
        host_str = ''
        for h in range(hosts_per_leaf):
            host_str += 'h%d' % host_num
            host_num += 1
        print('[%s]' % host_str.center(4), end='  ')
    print('\n')
    
    print('\nCharacteristics:')
    print('  ✓ Full mesh between spine and leaf switches')
    print('  ✓ Each host connected to one leaf switch')
    print('  ✓ Scalable: Can add more spine/leaf switches')
    print('  ✓ Redundant paths for high availability')
    print('  ✓ Predictable latency and bandwidth\n')


def create_custom_sizes():
    """Example: Create different sized topologies"""
    
    print('═' * 60)
    print('Available Topology Configurations:')
    print('═' * 60)
    print('1. Small  - 2 Spine, 2 Leaf, 2 Hosts/Leaf (Total: 4 hosts)')
    print('2. Medium - 2 Spine, 4 Leaf, 2 Hosts/Leaf (Total: 8 hosts)')
    print('3. Large  - 4 Spine, 8 Leaf, 4 Hosts/Leaf (Total: 32 hosts)')
    print('4. Custom - Specify your own parameters')
    print('═' * 60)
    
    choice = input('Enter choice (1-4): ')
    
    if choice == '1':
        run_topology(2, 2, 2)
    elif choice == '2':
        run_topology(2, 4, 2)
    elif choice == '3':
        run_topology(4, 8, 4)
    elif choice == '4':
        spine = int(input('Number of spine switches: '))
        leaf = int(input('Number of leaf switches: '))
        hosts = int(input('Hosts per leaf: '))
        run_topology(spine, leaf, hosts)
    else:
        print('Invalid choice, using default (2,4,2)')
        run_topology(2, 4, 2)


if __name__ == '__main__':
    setLogLevel('info')
    
    # Check if running with arguments
    import sys
    if len(sys.argv) == 4:
        # Run with command line arguments
        # Usage: sudo python leaf_spine_topology.py <spine> <leaf> <hosts_per_leaf>
        spine = int(sys.argv[1])
        leaf = int(sys.argv[2])
        hosts = int(sys.argv[3])
        run_topology(spine, leaf, hosts)
    elif len(sys.argv) == 1:
        # Interactive mode
        create_custom_sizes()
    else:
        print('Usage: sudo python leaf_spine_topology.py [spine] [leaf] [hosts_per_leaf]')
        print('   Or: sudo python leaf_spine_topology.py (for interactive mode)')
        print('\nExample: sudo python leaf_spine_topology.py 2 4 2')
