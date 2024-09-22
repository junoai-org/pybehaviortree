import behavior_tree_cpp as bt

# Definir funciones de Python para las acciones y condiciones
def check_condition():
    print("Checking condition in Python")
    return "SUCCESS"

def action_a():
    print("Executing Action A")
    return "FAILURE"

def action_b():
    print("Executing Action B")
    return "SUCCESS"

def my_action():
    print("Executing My Action")
    return "SUCCESS"

# Crear una fábrica y registrar las acciones y condiciones
factory = bt.BehaviorTreeFactory()
factory.register_simple_condition("MyCondition", check_condition)
factory.register_simple_action("ActionA", action_a)
factory.register_simple_action("ActionB", action_b)
factory.register_simple_action("MyAction", my_action)

# Definir el árbol en XML
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
blackboard = bt.Blackboard.create()
# Crear el árbol desde el texto XML
tree = factory.create_tree_from_text(tree_xml, blackboard)

# Añadir un logger para ver la ejecución
logger = bt.StdCoutLogger(tree)

# Ejecutar el árbol
status = tree.tick_root()
print("Estado del árbol:", status)