aaditya@aadi:~/cn_assign/assignment 14$ sudo python3 leaf_spine_topology.py 2 4 2
/home/aaditya/cn_assign/assignment 14/leaf_spine_topology.py:3: SyntaxWarning: invalid escape sequence '\ '
  """
╔═══════════════════════════════════════════════════════════╗
║     Leaf-Spine Topology - Assignment 14                  ║
╠═══════════════════════════════════════════════════════════╣
║  Configuration:                                           ║
║    Spine Switches: 2                                    ║
║    Leaf Switches:  4                                    ║
║    Hosts per Leaf: 2                                    ║
╚═══════════════════════════════════════════════════════════╝

*** Creating spine switches
  Added spine1
  Added spine2
*** Creating leaf switches and connecting to spines
  Added leaf1
    Connected leaf1 <-> spine1
    Connected leaf1 <-> spine2
  Added leaf2
    Connected leaf2 <-> spine1
    Connected leaf2 <-> spine2
  Added leaf3
    Connected leaf3 <-> spine1
    Connected leaf3 <-> spine2
  Added leaf4
    Connected leaf4 <-> spine1
    Connected leaf4 <-> spine2
*** Creating hosts and connecting to leaf switches
  Added h1 connected to leaf1
  Added h2 connected to leaf1
  Added h3 connected to leaf2
  Added h4 connected to leaf2
  Added h5 connected to leaf3
  Added h6 connected to leaf3
  Added h7 connected to leaf4
  Added h8 connected to leaf4
*** Topology creation complete
  Total Spine Switches: 2
  Total Leaf Switches: 4
  Total Hosts: 8
  Total Links: 16
*** Creating network
*** Adding controller
*** Adding hosts:
h1 h2 h3 h4 h5 h6 h7 h8 
*** Adding switches:
leaf1 leaf2 leaf3 leaf4 spine1 spine2 
*** Adding links:
(h1, leaf1) (h2, leaf1) (h3, leaf2) (h4, leaf2) (h5, leaf3) (h6, leaf3) (h7, leaf4) (h8, leaf4) (leaf1, spine1) (leaf1, spine2) (leaf2, spine1) (leaf2, spine2) (leaf3, spine1) (leaf3, spine2) (leaf4, spine1) (leaf4, spine2) 
*** Configuring hosts
h1 h2 h3 h4 h5 h6 h7 h8 
*** Starting network
*** Starting controller
c0 
*** Starting 6 switches
leaf1 leaf2 leaf3 leaf4 spine1 spine2 ...

*** Network Information:
Hosts: ['h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'h7', 'h8']
Switches: ['leaf1', 'leaf2', 'leaf3', 'leaf4', 'spine1', 'spine2']

*** Testing connectivity (pingAll)
*** Ping: testing ping reachability
h1 -> X X X X X X X 
h2 -> X X X X X X X 
h3 -> X X X X X X X 
h4 -> X X X X X X X 
h5 -> X X X X X X X 
h6 -> X X X X X X X 
h7 -> X X X X X X X 
h8 -> X X X X X X X 
*** Results: 100% dropped (0/56 received)

╔═══════════════════════════════════════════════════════════╗
║                  TOPOLOGY DIAGRAM                         ║
╚═══════════════════════════════════════════════════════════╝

Spine Layer (Core):
  [Spine1]  [Spine2]  
     |  |     |  |     
     ╲  ╱     ╲  ╱     

Leaf Layer (Access):
  [Leaf1]  [Leaf2]  [Leaf3]  [Leaf4]  
    ||      ||      ||      ||    

Host Layer:
  [h1h2]  [h3h4]  [h5h6]  [h7h8]  


Characteristics:
  ✓ Full mesh between spine and leaf switches
  ✓ Each host connected to one leaf switch
  ✓ Scalable: Can add more spine/leaf switches
  ✓ Redundant paths for high availability
  ✓ Predictable latency and bandwidth


*** Running CLI (type "help" for commands)
*** Useful commands:
    pingall          - Test connectivity between all hosts
    iperf h1 h2      - Test bandwidth between h1 and h2
    xterm h1 h2      - Open terminals for h1 and h2
    dump             - Display network information
    net              - Display network links

*** Starting CLI:
mininet> 

