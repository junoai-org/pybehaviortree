// behavior_tree_wrapper.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <memory>

// Incluir las cabeceras de BehaviorTree.CPP
#include <behaviortree_cpp_v3/bt_factory.h>
#include <behaviortree_cpp_v3/tree_node.h>
#include <behaviortree_cpp_v3/basic_types.h>
#include <behaviortree_cpp_v3/loggers/bt_cout_logger.h>
#include <behaviortree_cpp_v3/actions/always_success_node.h>
#include <behaviortree_cpp_v3/controls/sequence_node.h>
#include <behaviortree_cpp_v3/controls/fallback_node.h>
#include <behaviortree_cpp_v3/controls/parallel_node.h>
#include <behaviortree_cpp_v3/decorators/keep_running_until_failure_node.h>
#include <behaviortree_cpp_v3/decorator_node.h>
// Agrega otras cabeceras según necesites

namespace py = pybind11;
using namespace BT;

// Clase para envolver acciones definidas en Python
class PySyncActionNode : public SyncActionNode {
public:
    PySyncActionNode(const std::string& name, const py::object& py_func)
        : SyncActionNode(name, {}), _py_func(py_func) {}

    NodeStatus tick() override {
        try {
            // Access the blackboard
            auto blackboard = config().blackboard;
            py::object py_blackboard = py::cast(blackboard);

            // Call the Python function with the blackboard
            auto result = _py_func(py_blackboard);

            // Process the result
            if (py::isinstance<py::str>(result)) {
                std::string status = result.cast<std::string>();
                if (status == "SUCCESS") return NodeStatus::SUCCESS;
                if (status == "FAILURE") return NodeStatus::FAILURE;
                if (status == "RUNNING") return NodeStatus::RUNNING;
            } else if (py::isinstance<py::int_>(result)) {
                int status = result.cast<int>();
                return static_cast<NodeStatus>(status);
            }
        } catch (py::error_already_set& e) {
            std::cerr << "Exception in Python function: " << e.what() << std::endl;
        }
        return NodeStatus::FAILURE; // Default value
    }


private:
    py::object _py_func;
};

PYBIND11_MODULE(behavior_tree_cpp, m) {
    m.doc() = "Python wrapper for BehaviorTree.CPP";

    // Exponer enumeraciones
    py::enum_<NodeStatus>(m, "NodeStatus")
        .value("SUCCESS", NodeStatus::SUCCESS)
        .value("FAILURE", NodeStatus::FAILURE)
        .value("RUNNING", NodeStatus::RUNNING)
        .value("IDLE", NodeStatus::IDLE)
        .export_values();

    // Exponer la clase TreeNode
    py::class_<TreeNode, std::shared_ptr<TreeNode>>(m, "TreeNode")
        .def("name", &TreeNode::name)
        .def("status", &TreeNode::status)
        // .def("set_status", &TreeNode::setStatus) // Línea eliminada
        .def("type", &TreeNode::type)
        .def("halt", &TreeNode::halt)
        ;

    // Exponer los nodos de control (sequences, selectors, etc.)
    py::class_<ControlNode, TreeNode, std::shared_ptr<ControlNode>>(m, "ControlNode");

    py::class_<SequenceNode, ControlNode, std::shared_ptr<SequenceNode>>(m, "SequenceNode")
        .def(py::init<const std::string&>())
        ;

    py::class_<FallbackNode, ControlNode, std::shared_ptr<FallbackNode>>(m, "FallbackNode")
        .def(py::init<const std::string&>())
        ;

    py::class_<ParallelNode, ControlNode, std::shared_ptr<ParallelNode>>(m, "ParallelNode")
        .def(py::init<const std::string&, int>())
        ;

    // Exponer nodos de acción
    py::class_<ActionNodeBase, TreeNode, std::shared_ptr<ActionNodeBase>>(m, "ActionNodeBase");

    py::class_<SyncActionNode, ActionNodeBase, std::shared_ptr<SyncActionNode>>(m, "SyncActionNode");

    py::class_<PySyncActionNode, SyncActionNode, std::shared_ptr<PySyncActionNode>>(m, "PySyncActionNode")
        .def(py::init<const std::string&, const py::object&>())
        ;

    // Exponer nodos de decorador
    py::class_<DecoratorNode, TreeNode, std::shared_ptr<DecoratorNode>>(m, "DecoratorNode");

    py::class_<KeepRunningUntilFailureNode, DecoratorNode, std::shared_ptr<KeepRunningUntilFailureNode>>(m, "KeepRunningUntilFailureNode")
        .def(py::init<const std::string&>());

    // Exponer Tree
    py::class_<Tree>(m, "Tree")
        .def("tick_root", &Tree::tickRoot)
        .def("root_node", [](Tree& tree) {
            return tree.rootNode();
        }, py::return_value_policy::reference)
        .def("blackboard", &Tree::rootBlackboard)
        .def("halt_tree", &Tree::haltTree)
        ;

    // Exponer BehaviorTreeFactory y registrar los nodos incorporados
    py::class_<BehaviorTreeFactory>(m, "BehaviorTreeFactory")
        .def(py::init<>())
        .def("register_from_file", &BehaviorTreeFactory::registerBehaviorTreeFromFile)
        .def("register_from_text", &BehaviorTreeFactory::registerBehaviorTreeFromText)
        .def("create_tree_from_text",
         [](BehaviorTreeFactory& self, const std::string& text, py::object blackboard = py::none()) {
             if (!blackboard.is_none()) {
                 auto bb = blackboard.cast<std::shared_ptr<Blackboard>>();
                 return self.createTreeFromText(text, bb);
             } else {
                 return self.createTreeFromText(text); // Usa Blackboard predeterminado
             }
         })
        .def("create_tree_from_file",
         [](BehaviorTreeFactory& self, const std::string& filename, py::object blackboard = py::none()) {
             if (!blackboard.is_none()) {
                 auto bb = blackboard.cast<std::shared_ptr<Blackboard>>();
                 return self.createTreeFromFile(filename, bb);
             } else {
                 return self.createTreeFromFile(filename); // Usa Blackboard predeterminado
             }
         })
        .def("register_simple_action", [](BehaviorTreeFactory& factory, const std::string& name, const py::function& py_func) {
            factory.registerSimpleAction(name, [py_func](TreeNode& self) -> NodeStatus {
                try {
                    // Access the blackboard
                    auto blackboard = self.config().blackboard;
                    py::object py_blackboard = py::cast(blackboard);

                    // Call the Python function with the blackboard
                    auto result = py_func(py_blackboard);

                    // Process the result
                    if (py::isinstance<py::str>(result)) {
                        std::string status = result.cast<std::string>();
                        if (status == "SUCCESS") return NodeStatus::SUCCESS;
                        if (status == "FAILURE") return NodeStatus::FAILURE;
                        if (status == "RUNNING") return NodeStatus::RUNNING;
                    } else if (py::isinstance<py::int_>(result)) {
                        int status = result.cast<int>();
                        return static_cast<NodeStatus>(status);
                    } else if (py::isinstance<py::bool_>(result)) {
                        bool status = result.cast<bool>();
                        return status ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
                    }
                } catch (py::error_already_set& e) {
                    std::cerr << "Exception in Python action: " << e.what() << std::endl;
                }
                return NodeStatus::FAILURE;
            });
        })
        .def("register_simple_condition", [](BehaviorTreeFactory& factory, const std::string& name, const py::function& py_func) {
            factory.registerSimpleCondition(name, [py_func](TreeNode& self) -> NodeStatus {
                try {
                    // Access the blackboard
                    auto blackboard = self.config().blackboard;
                    py::object py_blackboard = py::cast(blackboard);

                    auto result = py_func(py_blackboard);
                    if (py::isinstance<py::str>(result)) {
                        std::string status = result.cast<std::string>();
                        if (status == "SUCCESS") return NodeStatus::SUCCESS;
                        if (status == "FAILURE") return NodeStatus::FAILURE;
                        if (status == "RUNNING") return NodeStatus::RUNNING;
                    } else if (py::isinstance<py::int_>(result)) {
                        int status = result.cast<int>();
                        return static_cast<NodeStatus>(status);
                    }
                } catch (py::error_already_set& e) {
                    std::cerr << "Exception in Python condition: " << e.what() << std::endl;
                }
                return NodeStatus::FAILURE;
            });
        })
        // Registrar nodos incorporados
        .def("register_node", [](BehaviorTreeFactory& factory, const std::string& name, const std::string& type) {
            if (type == "Sequence") {
                factory.registerNodeType<SequenceNode>(name);
            } else if (type == "Fallback") {
                factory.registerNodeType<FallbackNode>(name);
            } else if (type == "Parallel") {
                factory.registerNodeType<ParallelNode>(name);
            } else if (type == "KeepRunningUntilFailure") {
                factory.registerNodeType<KeepRunningUntilFailureNode>(name);
            } else {
                throw std::runtime_error("Unknown node type: " + type);
            }
            // Agrega más tipos si es necesario
        })
        ;

    // Exponer el logger
    py::class_<StdCoutLogger>(m, "StdCoutLogger")
        .def(py::init<Tree&>())
        ;

    // Expose the blackboard creation with optional parent
    py::class_<Blackboard, std::shared_ptr<Blackboard>>(m, "Blackboard")
        .def_static(
            "create",
            [](py::object parent_bb = py::none()) {
                if (!parent_bb.is_none()) {
                    auto parent = parent_bb.cast<std::shared_ptr<Blackboard>>();
                    return Blackboard::create(parent);
                } else {
                    return Blackboard::create();
                }
            },
            py::arg("parent_bb") = py::none()
        )
        .def("set", [](Blackboard& self, const std::string& key, const py::object& value) {
            self.set<py::object>(key, value);
        })
        .def("get", [](Blackboard& self, const std::string& key) -> py::object {
            return self.get<py::object>(key);
        })
        .def("get_keys", [](Blackboard& self) {
            return self.getKeys();
        });

}
