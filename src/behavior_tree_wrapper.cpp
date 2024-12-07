#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <memory>

#include <behaviortree_cpp_v3/bt_factory.h>
#include <behaviortree_cpp_v3/tree_node.h>
#include <behaviortree_cpp_v3/basic_types.h>
#include <behaviortree_cpp_v3/loggers/bt_file_logger.h>
#include <behaviortree_cpp_v3/loggers/bt_cout_logger.h>
#include <behaviortree_cpp_v3/actions/always_success_node.h>
#include <behaviortree_cpp_v3/controls/sequence_node.h>
#include <behaviortree_cpp_v3/controls/fallback_node.h>
#include <behaviortree_cpp_v3/controls/parallel_node.h>
#include <behaviortree_cpp_v3/decorators/keep_running_until_failure_node.h>
#include <behaviortree_cpp_v3/decorator_node.h>

namespace py = pybind11;
using namespace BT;

static NodeStatus pyresult_to_status(const py::object& result) {
    if (py::isinstance<py::str>(result)) {
        auto status = result.cast<std::string>();
        if (status == "SUCCESS") return NodeStatus::SUCCESS;
        if (status == "FAILURE") return NodeStatus::FAILURE;
        if (status == "RUNNING") return NodeStatus::RUNNING;
    } else if (py::isinstance<py::int_>(result)) {
        int st = result.cast<int>();
        return static_cast<NodeStatus>(st);
    } else if (py::isinstance<py::bool_>(result)) {
        bool st = result.cast<bool>();
        return st ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }
    return NodeStatus::FAILURE;
}

// Clase para envolver acciones definidas en Python
class PySyncActionNode : public SyncActionNode {
public:
    PySyncActionNode(const std::string& name, const py::object& py_func)
        : SyncActionNode(name, {}), _py_func(py_func) {}

    NodeStatus tick() override {
        py::gil_scoped_acquire acquire;
        try {
            auto blackboard = config().blackboard;
            py::object py_blackboard = py::cast(blackboard);
            auto result = _py_func(py_blackboard);
            return pyresult_to_status(result);
        } catch (py::error_already_set& e) {
            std::cerr << "Exception in Python function for node [" << name() << "]: " << e.what() << std::endl;
            return NodeStatus::FAILURE;
        }
    }

private:
    py::object _py_func;
};

PYBIND11_MODULE(behavior_tree_cpp, m) {
    m.doc() = "Python wrapper for BehaviorTree.CPP";

    py::enum_<NodeStatus>(m, "NodeStatus")
        .value("SUCCESS", NodeStatus::SUCCESS)
        .value("FAILURE", NodeStatus::FAILURE)
        .value("RUNNING", NodeStatus::RUNNING)
        .value("IDLE", NodeStatus::IDLE)
        .export_values();

    py::class_<TreeNode, std::shared_ptr<TreeNode>>(m, "TreeNode")
        .def("name", &TreeNode::name)
        .def("status", &TreeNode::status)
        .def("type", &TreeNode::type)
        .def("halt", &TreeNode::halt);

    py::class_<ControlNode, TreeNode, std::shared_ptr<ControlNode>>(m, "ControlNode");

    py::class_<SequenceNode, ControlNode, std::shared_ptr<SequenceNode>>(m, "SequenceNode")
        .def(py::init<const std::string&>());

    py::class_<FallbackNode, ControlNode, std::shared_ptr<FallbackNode>>(m, "FallbackNode")
        .def(py::init<const std::string&>());

    py::class_<ParallelNode, ControlNode, std::shared_ptr<ParallelNode>>(m, "ParallelNode")
        .def(py::init<const std::string&, int>());

    py::class_<ActionNodeBase, TreeNode, std::shared_ptr<ActionNodeBase>>(m, "ActionNodeBase");
    py::class_<SyncActionNode, ActionNodeBase, std::shared_ptr<SyncActionNode>>(m, "SyncActionNode");

    py::class_<PySyncActionNode, SyncActionNode, std::shared_ptr<PySyncActionNode>>(m, "PySyncActionNode")
        .def(py::init<const std::string&, const py::object&>());

    py::class_<DecoratorNode, TreeNode, std::shared_ptr<DecoratorNode>>(m, "DecoratorNode");
    py::class_<KeepRunningUntilFailureNode, DecoratorNode, std::shared_ptr<KeepRunningUntilFailureNode>>(m, "KeepRunningUntilFailureNode")
        .def(py::init<const std::string&>());

    py::class_<Tree>(m, "Tree")
        .def("tick_root", &Tree::tickRoot)
        .def("root_node", [](Tree& tree) {
            return tree.rootNode();
        }, py::return_value_policy::reference)
        .def("blackboard", &Tree::rootBlackboard)
        .def("halt_tree", &Tree::haltTree);

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
                     return self.createTreeFromText(text);
                 }
             })
        .def("create_tree_from_file",
             [](BehaviorTreeFactory& self, const std::string& filename, py::object blackboard = py::none()) {
                 if (!blackboard.is_none()) {
                     auto bb = blackboard.cast<std::shared_ptr<Blackboard>>();
                     return self.createTreeFromFile(filename, bb);
                 } else {
                     return self.createTreeFromFile(filename);
                 }
             })
        .def("register_simple_action", [](BehaviorTreeFactory& factory, const std::string& name, const py::function& py_func) {
            factory.registerSimpleAction(name, [py_func](TreeNode& self) -> NodeStatus {
                py::gil_scoped_acquire acquire;
                try {
                    auto blackboard = self.config().blackboard;
                    py::object py_blackboard = py::cast(blackboard);
                    auto result = py_func(py_blackboard);
                    return pyresult_to_status(result);
                } catch (py::error_already_set& e) {
                    std::cerr << "Exception in Python action [" << self.name() << "]: " << e.what() << std::endl;
                    return NodeStatus::FAILURE;
                }
            });
        })
        .def("register_simple_condition", [](BehaviorTreeFactory& factory, const std::string& name, const py::function& py_func) {
            factory.registerSimpleCondition(name, [py_func](TreeNode& self) -> NodeStatus {
                py::gil_scoped_acquire acquire;
                try {
                    auto blackboard = self.config().blackboard;
                    py::object py_blackboard = py::cast(blackboard);
                    auto result = py_func(py_blackboard);
                    return pyresult_to_status(result);
                } catch (py::error_already_set& e) {
                    std::cerr << "Exception in Python condition [" << self.name() << "]: " << e.what() << std::endl;
                    return NodeStatus::FAILURE;
                }
            });
        })
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
        });

    py::class_<StdCoutLogger>(m, "StdCoutLogger")
        .def(py::init<Tree&>());

    // Expose the StatusChangeLogger class
    py::class_<BT::StatusChangeLogger, std::shared_ptr<BT::StatusChangeLogger>>(m, "StatusChangeLogger")
        .def("callback", &BT::StatusChangeLogger::callback)
        .def("flush", &BT::StatusChangeLogger::flush);

    py::class_<BT::FileLogger, BT::StatusChangeLogger, std::shared_ptr<BT::FileLogger>>(m, "FileLogger")
        .def(py::init<const BT::Tree&, const char*, uint16_t>(),
             py::arg("tree"),
             py::arg("filepath"),
             py::arg("buffer_size") = 10)
        .def(py::init([](const BT::Tree& tree, const std::string& filepath, uint16_t buffer_size) {
            return std::make_shared<BT::FileLogger>(tree, filepath.c_str(), buffer_size);
        }));

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
