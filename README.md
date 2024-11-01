# PyBehaviorTree

PyBehaviorTree is a Python wrapper for the [BehaviorTree.CPP](https://github.com/BehaviorTree/BehaviorTree.CPP) library, allowing developers to create and manipulate behavior trees in Python using the powerful features offered by BehaviorTree.CPP.

**Note:** This project is currently in **active development (Work In Progress - WIP)**. Functionalities and APIs are subject to change.

## Description

Behavior trees are a powerful tool for decision-making in artificial intelligence, used in areas such as video games, robotics, and automation applications. While BehaviorTree.CPP provides a robust implementation in C++, PyBehaviorTree aims to bring this functionality to Python developers.

## Features

- **Integration with pybind11:** Uses pybind11 to expose BehaviorTree.CPP classes and methods to Python.
- **Tree definition in XML or text:** Load and execute behavior trees defined in XML files or text strings.
- **Registration of custom actions and conditions:** Create and register action and condition nodes using Python functions.
- **Support for control nodes:** Use sequences, selectors, and parallel nodes in your behavior trees.
- **Communication through Blackboard:** Share information between nodes using the Blackboard pattern.

## Installation

### Prerequisites

- Python 3.x
- C++17 compatible compiler
- Git
- CMake
- For Linux: libzmq3-dev and libboost-dev

### Installation Steps

1. **Clone the PyBehaviorTree repository with submodules:**

   ```bash
   git clone --recursive https://github.com/junoai-org/pybehaviortree
   cd PyBehaviorTree
   ```

   This will clone the repository and initialize the BehaviorTree.CPP and pybind11 submodules.

2. **If you've already cloned the repository without --recursive, initialize the submodules:**

   ```bash
   git submodule update --init --recursive
   ```

3. **Install required dependencies (for Linux):**

   ```bash
   sudo apt-get install libzmq3-dev libboost-dev
   ```

4. **Compile BehaviorTree.CPP:**

   ```bash
   cd BehaviorTree.CPP
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   cd ../..  # Return to the PyBehaviorTree root directory
   ```

5. **Compile the PyBehaviorTree project:**

   ```bash
   python setup.py build
   ```

6. **Install PyBehaviorTree:**

   ```bash
   python setup.py install --user
   ```

## Usage Example

```python
import behavior_tree_cpp as bt

# Define Python functions for actions and conditions
def check_condition(blackboard):
    print("Checking condition in Python")
    # Condition logic
    return bt.NodeStatus.SUCCESS

def action_a(blackboard):
    print("Executing Action A")
    # Action logic
    return bt.NodeStatus.FAILURE

def action_b(blackboard):
    print("Executing Action B")
    # Action logic
    return bt.NodeStatus.SUCCESS

def my_action(blackboard):
    print("Executing My Action")
    # Action logic
    return bt.NodeStatus.SUCCESS

# Create a factory and register actions and conditions
factory = bt.BehaviorTreeFactory()
factory.register_simple_condition("MyCondition", check_condition)
factory.register_simple_action("ActionA", action_a)
factory.register_simple_action("ActionB", action_b)
factory.register_simple_action("MyAction", my_action)

# Define the tree in XML format
tree_xml = """
<root main_tree_to_execute="MainTree">
    <BehaviorTree ID="MainTree">
        <Sequence name="MainSequence">
            <MyCondition/>
            <Fallback name="FallbackNode">
                <ActionA/>
                <ActionB/>
            </Fallback>
            <MyAction/>
        </Sequence>
    </BehaviorTree>
</root>
"""

# Create the tree from XML text
blackboard = bt.Blackboard.create()
tree = factory.create_tree_from_text(tree_xml, blackboard)

# Add a logger to visualize execution
logger = bt.StdCoutLogger(tree)

# Execute the tree
status = tree.tick_root()
print(f"Tree status: {status}")
```

## Project Status

This project is in the development phase and may not be fully functional. We appreciate your understanding and patience as we continue to work on improving and expanding the functionalities.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more information.
