
import sys
from collections import deque
from copy import deepcopy
import networkx as nx
import matplotlib.pyplot as plt
from datetime import datetime






LOGIC3_1 = 1
LOGIC3_HALF = 0.5
LOGIC3_0 = 0



FAILSTATE_NAME = "fail"
NULL = "NULL"


TRANSFORMER_ASSIGN_VAR = "assign_var"
TRANSFORMER_ASSIGN_NEXT = "assign_next"
TRANSFORMER_NEXT_ASSIGN = "next_assign"
TRANSFORMER_ASSUME = "assume"
TRANSFORMER_ASSERT = "assert"
TRANSFORMER_NEW = "new"

EXPR_VAREQNULL = "var==NULL"
EXPR_VARNEQNULL = "var!=NULL"
EXPR_VAREQVAR = "var==var"
EXPR_VARNEQVAR = "var!=var"
EXPR_VAREQVARNEXT = "var==var.n"
EXPR_VARNEQVARNEXT = "var!=var.n"
EXPR_LS = "ls x y"
EXPR_ODD = "odd"
EXPR_EVEN = "even"
EXPR_LEN = "len x y == len z w"
EXPR_TRUE = "TRUE"
EXPR_FALSE = "FALSE"


def main():
    path = sys.path[0]
    try:
        # Add filename reading from sys.path
        filename = sys.argv[1]
        print ("Starting shape analysis on file:  " + filename)
        codefile = open(path + "\\" + filename, 'r')
        code_file_string = file.read(codefile)
        graph = Cfg(code_file_string)
        graph.analyze()
        print("Analysis complete!")
    except ErrorParsing:
        print("Error: File parsing error or illegal code given")
    except ErrorNullDereference:
        print("Error: NULL referenced by code (or maybe referenced)")
    except ErrorAssertionFailed:
        print("Error: Failed to properly assert a required assertion")
    except ErrorIllegalHeap:
        print ("Error: The heap analyzed had an illegal/illogical structure")




class Cfg:
    """
    Class for a control flow graph
    Fields:
        * dictionary of (state_name, State) - states
        * list of edges - edges
        * State fail state
        * State start state
        * list of variables - variables
    """
    def __init__(self, codestring):
        """
        Constructor
        
        @input: codefile - Whole string of the file containing the code to be analyzed
                           Should be divided by '\n' between lines and spaces between state label and commands
                           Proper syntax: $pre-state$ $command$ $post-state$\n
        """
        lines = codestring.split("\n")
        self.edges = []
        self.variables = ["NULL"]
        self.start_state = None
        self.fail_state = State("fail",self)
        self.states = {}
        self.states["fail"] = self.fail_state
        # Parse variables
        var_line = lines[0]
        for var in var_line.split():
            self.variables += [var]
        # Parse each line
        for line in lines[1:]:
            line_parts = [line[:line.find(" ")], line[line.find(" ")+1: line.rfind(" ")] , line[line.rfind(" ")+1:]]
            # if start uninitialized, set it
            if self.start_state == None:
                self.start_state = State(line_parts[0],self)
                self.start_state.heaps = [Heap(self.variables)]
                self.states[line_parts[0]] = self.start_state
            if line_parts[0] not in self.states.keys():
                self.states[line_parts[0]] = State(line_parts[0],self)
            if line_parts[2] not in self.states.keys():
                self.states[line_parts[2]] =  State(line_parts[2],self)
            new_edge = Edge(line_parts[0], line_parts[1], line_parts[2])
            self.edges.append(new_edge)
            self.states[line_parts[0]].out_edges.append(new_edge)
            self.states[line_parts[2]].in_edges.append(new_edge)
            # Add edge to fail from assert if necessary
            if "assert" in line_parts[1]:
                fail_edge = Edge(line_parts[0], line_parts[1], FAILSTATE_NAME )
                self.edges += [fail_edge] 
                self.states[FAILSTATE_NAME].in_edges += [fail_edge]
                self.states[line_parts[0]].out_edges += [fail_edge]
        

    def analyze(self):
        """
        Runs the analysis on the CFG.
        Uses disjunctive completion of possible heaps.
        Uses disjunctive completion of different heaps in every state.
        So assume/assert when condition doesn't hold adds nothing
        """
        
        try:
            worklist = set([self.start_state])
            while not len(worklist) == 0:
                current_src_state = worklist.pop()
                print("Analyzing state:" + current_src_state.name)
                for edge in current_src_state.out_edges:
                    transformed = []
                    src_heaps = current_src_state.heaps
                    if edge.op.operation == TRANSFORMER_ASSERT and edge.dst == FAILSTATE_NAME:
                        # For assertions, we need ALL the heaps in the source state to hold, otherwise its unprovable...
                        for heap in src_heaps:
                            focused = heap.focus(edge.op)
                            for h in focused:
                                heap_after_transformer = h.apply_transformer(edge.op)
                                # If the transformer resulted in None then the assertion does not hold for this heap
                                if heap_after_transformer == None:
                                    raise ErrorAssertionFailed()
                        print("Assertion Successfull!  " + edge.op.entire_command)
                        # Save picture #################
                        draw_state_to_png(current_src_state)
                        ################################
                    else:
                        for heap in src_heaps:
                            # Returns the heap after the transformer or None (if condition evaluated to 0)
                            focused = heap.focus(edge.op)
                            for h in focused:
                                transformed.append(h.apply_transformer(edge.op))  
                        # Remove all "None" heaps
                        transformed = [heap for heap in transformed if heap != None]
                        # Coerce results
                        transformed = coerce_red(transformed)
                        # Use canonical abstraction
                        canon_abst  = canonical_abstraction(transformed)
                        # If not all heaps in update are included in destination (disjunctive completion, all options already exist in edge.dst) update and add to worklist                
                        all_included = True
                        updated_state_heaps = heaps_join(self.states[edge.dst].heaps, canon_abst )
                        if len(updated_state_heaps) > len(self.states[edge.dst].heaps):
                            all_included = False
                        if all_included == False:
                            # Update destination by joining its heap with the transformed heap of the source
                            self.states[edge.dst].heaps = updated_state_heaps
                            worklist.add(self.states[edge.dst])
            print ("Reached fixed point, analysis complete")
            cd()
            print ("Saving last analyzed state, before command " + edge.op.entire_command)
            draw_state_to_png(current_src_state)
        except ErrorNullDereference:
            print("Error: NULL referenced by code (or maybe referenced)")
            print("Command was:" + edge.op.entire_command)
            draw_state_to_png(current_src_state)
        except ErrorAssertionFailed:
            print("Error: Failed to properly assert a required assertion")
            print("Command was:" + edge.op.entire_command)
            draw_state_to_png(current_src_state)
        except ErrorIllegalHeap:
            print ("Error: The heap analyzed had an illegal/illogical structure")

            
        
        
        
class State:
    """
    Class for a state in the control flow graph (Cfg)
    Fields:
        * list of Heap - heaps
        * list of Edge - out_edges    
        * list of Edge - in_edges
        * Cfg - the containing graph (pointer)
    """
      
    def __init__(self,name, graph):
        """
        Constructor
        Creates a state with an empty heap
        @input: string name
                list of Edge - e_out
                list of Edge - e_in
                Cfg - graph_ptr
        """
        self.name = name
        self.out_edges = []
        self.in_edges = []
        self.heaps = []
        self.graph_ptr = graph
            
class Edge:
    """
    Class for a control flow graph
    Fields:
        * State - src 
        * State - dst 
        * Transformer - op  
    """
      
    def __init__(self, source, operation, destination):
        """
        Constructor
        Creates a node with all lattices at bottom.
        @input: 
        """
        self.src = source
        self.dst = destination
        self.op = Transformer(operation)
        
        
class Heap:
    """
    Class for a (partially) concrete node on the heap, i.e. , a collection of predicates describing a heap
    Fields:
        * 3-value logic predicates:
            * dictionary of var_name to unary predicate (dictionary of node to 3-val bool) - var_pts_to
            * dictionary of node to dictionary of node to 3-val bool (predecessor, next) - next
            * unary predicate. dictionary of node to 3-val bool - is_summary
            * dictionary of var_name to unary predicate (dictionary of node to 3-val bool) - reachable
            * unary predicate. dictionary of node to 3-val bool - in_cycle
            * unary predicate. dictionary of node to 3-val bool - is_shared
        * int max_heap_index = integer to upper bound on all heap addresses.
                               Used to draw heap addresses with 'new'
        * heap_items = list of indexes/addresses used in this heap. Does not garbage-collect
    """
      
    def __init__(self, variables):
        """
        Constructor
        @input: variables = list of variables to include
        """
        self.var_pts_to = {}
        self.reachable = {}
        # Point all variables to NULL in this new heap
        for var in variables:
            self.var_pts_to[var] = {NULL: LOGIC3_1}
            self.reachable[var] = {NULL: LOGIC3_1}
        self.next = {NULL : { NULL : LOGIC3_0}}
        self.is_summary = {NULL : LOGIC3_0}
        self.in_cycle = {NULL : LOGIC3_0}
        self.is_shared = {NULL : LOGIC3_1}
        self.max_heap_index = 0
        self.heap_items = [NULL]
        
        
    def copy_Heap(self, other):
        """
        Copies all properties of 'other' to this. 
        @input: Heap other
        @output:
        """
        self.var_pts_to = deepcopy(other.var_pts_to)
        self.next = deepcopy(other.next)
        self.is_summary = deepcopy(other.is_summary)
        self.reachable = deepcopy(other.reachable)
        self.in_cycle = deepcopy(other.in_cycle)
        self.is_shared = deepcopy(other.is_shared)
        self.max_heap_index = deepcopy(other.max_heap_index)
        self.heap_items = deepcopy(other.heap_items)
        
    
    
    def apply_transformer(self, transformer):
        """
        Applies the input transformer onto the state, returning a the state after the transformer
        @input: Transformer transformer = The transformer to be applied

        @output: Heap      = transformer('this')
        
        Notes: does not update 'this' according to transformer
        """
        self_copy = Heap(self.var_pts_to.keys())
        self_copy.copy_Heap(self)
        if transformer.operation == TRANSFORMER_ASSERT:
            ans = self_copy.transformer_assert(transformer.expression)
            # Returns None if condition did not hold
        elif transformer.operation == TRANSFORMER_ASSIGN_NEXT:
            ans = self_copy.transformer_assign_next(transformer.arg1, transformer.arg2)
        elif transformer.operation == TRANSFORMER_ASSIGN_VAR:
            ans = self_copy.transformer_assign_var(transformer.arg1, transformer.arg2)
        elif transformer.operation == TRANSFORMER_ASSUME:
            ans = self_copy.transformer_assume(transformer.expression)
            # Returns None if condition did not hold
        elif transformer.operation == TRANSFORMER_NEW:
            ans = self_copy.transformer_new(transformer.arg1)
        elif transformer.operation == TRANSFORMER_NEXT_ASSIGN:
            ans = self_copy.transformer_next_assign(transformer.arg1, transformer.arg2)
        else:
            raise Exception()
        return ans
        
    def transformer_assign_var(self, var_assignee, var_assigned):
        """
        var_assignee := var_assigned
        Applies the transformer onto 'this'
        @input: string var_assignee = name of the variable edited
                string var_assigned = name of the variable being copied to the assignee
        @output: 'this'
        Notes: implements transformer on "self"
        """
        # assigned_ptr_predicate = self.var_pts_to[var_assigned]
        # self.var_pts_to[var_assignee] = deepcopy(assigned_ptr_predicate)
        self.set_var(var_assignee,self.get(var_assigned))
        
        return self
  
    def transformer_assign_next(self, x, t):
        """
        x := t.next
        Applies the transformer onto 'this' by setting x to point to t.next
        @input: string x = name of the variable edited
                string t = name of the variable who's ancestor is being set
        @output: 'this'
        Notes: implements transformer on "self"
        """
        # Get where arg2 points to
        pred_node = self.get(t)
        if pred_node == NULL:
            raise ErrorNullDereference()
        # Set assignee to point to pred_node.next
        self.set_var(x, self.get_next(pred_node))
        return self
        
    def transformer_next_assign(self, x, t):
        """
        x.n:=t
        Applies the transformer onto 'this' by setting where var_assignee_pred's "next" points to the node var_assigned points to
        @input: string var_assignee_pred = variable who's "next" ptr is being edited
                string var_assigned = name of the variable who is set to point to the ancestor
        @output: 'this'
        Notes: implements transformer on "self"
        """
                
        # Zeroize where var_assignee_pred.next points to
        at_x = self.get(x)
        at_t = self.get(t)
        # Disconnect current '.n' fields from *x
        for addr in self.next[at_x].keys():
            self.next[at_x][addr] = LOGIC3_0
        # Set the next to point where it should
        self.next[at_x][at_t] = LOGIC3_1
        self.reachable[x][at_t] = LOGIC3_1
        # Update to shared if it is shared
        sharing_nodes = [v for v in self.heap_items if self.next[v][at_t] != LOGIC3_HALF]
        if len(sharing_nodes) > 1:
            definite = [v for v in self.heap_items if self.next[v][at_t] == LOGIC3_1]
            if len(definite) > 1:
                self.is_shared[at_t] = LOGIC3_1
            else:
                self.is_shared[at_t] = LOGIC3_HALF
        
        
        # Update reachability: remap reachability for every variable to every item
        for y in self.var_pts_to.keys():
            for u in self.heap_items:
                self.reachable[y][u] = self.check_reachable(self.get(y), u)
        # Check if cycle is formed, update accordingly.
        for v in self.heap_items:
            self.in_cycle[v] = self.check_cycle(v)
        return self
        
    def transformer_new (self,var_assignee):
        """
        var_assignee := new
        Applies the transformer onto 'this' by adding a new heap address to the heap and setting var_assignee to point to it
        @input: string var_assignee = name of variable assigned to the new heap address
        @output: 'this'
        Notes: implements transformer on "self"
        """
        # Create new heap address
        new_node = self.new_node()
        # Set var_assignee to point to it. set_var redirects already
        self.set_var(var_assignee, new_node)
        return self
        
    def transformer_assume(self,expression):
        """
        assume(expr)
        Returns the same heap if expression holds or evaluates to half. Otherwise, returns None
        @input: Expression expression - the expression to evaluate
        @output: 'this'
        Notes: implements transformer on "self"
        """
        expr_evaluated_to = expression.evaluate(self)
        if expr_evaluated_to != LOGIC3_0:
            return self
        else:
            return None
        
    def transformer_assert(self,expression):
        expr_evaluated_to = expression.evaluate(self)
        if expr_evaluated_to == LOGIC3_1:
            return self
        else:
            # evaluation of 1/2 is not enough for assert
            return None
    
    def get(self, var):
        """
        Returns the heap address this var is assigned to from the vars_pts_to predicates
        @input: string var = variable to lookup
        @output: node = node this variable points to
        """
        # Get all heap addresses who have a 1 boolean value
        if var == "NULL":
            return NULL
        this_var_pts_to_list = [addr for addr in self.var_pts_to[var] if self.var_pts_to[var][addr] == LOGIC3_1]
        if len(this_var_pts_to_list) != 1:
            raise ErrorIllegalHeap()
        return this_var_pts_to_list[0]
    
    def get_next(self, pred_node):
        """
        Returns the heap address in pred_node.next
        @input: node pred_node
        @output: node = pred_node.next
        """
        next_pts_to_list = [addr for addr in self.next[pred_node].keys() if self.next[pred_node][addr] == LOGIC3_1]
        # There's surely a definite next because we used FOCUS for any operation using get.next
        if len(next_pts_to_list) != 1:
            raise ErrorIllegalHeap()
        return next_pts_to_list[0]
    
    def is_mergable(self, heap_arg1, heap_arg2):
        """
        Checks whether in this heap heap_arg1 and heap_arg2 have the same abstraction properties
        Our abstraction properties are:
            \forall x in vars: x(v) 
            is_shared(v)
            in_cycle(v)
            
        @input: heap_adress heap_arg1
                heap_adress heap_arg2
        @output: bool
        """
        # NULL cannot be summarized with anything
        if heap_arg1 == NULL or heap_arg2 == NULL:
            return False
        mergable = True
        for x in self.var_pts_to.keys():
            if self.var_pts_to[x][heap_arg1] != self.var_pts_to[x][heap_arg2]:
                mergable = False
                return mergable
            if self.reachable[x][heap_arg1] != self.reachable[x][heap_arg2]:
                mergable = False
                return mergable
        if self.is_shared[heap_arg1] != self.is_shared[heap_arg2]:
            mergable = False
        if self.in_cycle[heap_arg1] != self.in_cycle[heap_arg2]:
            mergable = False
        return mergable
        
    def merge(self, v1, v2):
        """
    Merges a v2 into v1 and updates predicates accordingly
    for consistency purposes, v1 is the lower address and v2 the higher address
    @input: heap item v1
            heap item v2
    @output: merged heap item
    """  
        if v1 == NULL or v2 == NULL:
            return NULL
        # Merging two nodes on the heap:
        #   Merge v2 into v1 using join
        #   Copy abstraction predicates from either (they are identical) - is(v), x(v), 
        #   Calculate instrumentation predicates from both
        # For every variable x, x(v1) is already set to the consistent value. If v1, v2 are mergable, they are identical. no change required
        # Calculate instrumentation predicates: r_x(v), next using 3-val logic JOIN , c(v)
        
        # Update reachable: every variable that reaches v1 or v2 now reaches every item reachable from v1 and every item reachable from v2
        # Updated value is surely 1/2, because inside the summary node the variable may or may not reach...        
        for x in self.var_pts_to.keys():
            self.reachable[x][v1] = or3(self.reachable[x][v1], self.reachable[x][v2])
        # Next
        for v in self.heap_items:
            # A summary never has a precise next. Turn all possible 'nexts' to at most 1/2s
            # A summary is never definitely anyone's next - since we don't know which in the summary
            self.next[v1][v] = and3 (LOGIC3_HALF, join3(self.next[v1][v] , self.next[v2][v]))
            self.next[v][v1] = and3 (LOGIC3_HALF, join3(self.next[v][v1] , self.next[v][v2]))
        # If now more than one pointer - its shared
        predecessors = [u for u in self.heap_items if self.next[u][v1]]
        if len(predecessors) > 1:
            self.is_shared[v1] = LOGIC3_HALF
        self.is_summary[v1] = LOGIC3_1
        self.next[v1][v1] = LOGIC3_HALF
        self.in_cycle[v1] = LOGIC3_HALF
        self.remove_node(v2)
        return v1
    
    def new_node(self):
        """
        Creates a new item in this heap and returns it
        @input: 
        @output:  the new node
                  updates 'this' accordingly
        """
        # Find first available "memory address" (id)
        new_node = 0
        for i in range(1,self.max_heap_index):
            if i not in self.heap_items:
                new_node = i
                break
        if new_node == 0:
            new_node = self.max_heap_index + 1
            self.max_heap_index += 1
        # Add to all variables that they don't point to the new one
        for x in self.var_pts_to.keys():
            self.var_pts_to[x][new_node] = LOGIC3_0
        # Add for new heap address that its next is NULL
        self.next[new_node] = { NULL :LOGIC3_1}
        for heap_addr in self.next.keys():
            if heap_addr != NULL:
                self.next[new_node][heap_addr] = LOGIC3_0
        # Add for each heap address that its next is NOT the new heap address
        for heap_addr in self.next.keys():
            self.next[heap_addr][new_node] = LOGIC3_0
        # Set new heap address as non-summary
        self.is_summary[new_node] = LOGIC3_0
        # Set as not reachable from any variable
        for x in self.var_pts_to.keys():
            self.reachable[x][new_node] =  LOGIC3_0
        # Set as not shared and not in a acycle
        self.is_shared[new_node] = LOGIC3_0
        self.in_cycle[new_node] = LOGIC3_0
        # Add to heap items
        self.heap_items += [new_node]
        return new_node

    def remove_node(self, v):
        """
        Removes node 'v' from the heap along with all its predicates. Used after a 'merge' command
        @input: v 
        @output: edits 'this'
        """
        # Remove from heap list
        self.heap_items = [item for item in self.heap_items if item != v]        
        # Remove from predicates: var_pts_to, next, reachable, cycle, is_summary, is_shared
        for x in self.var_pts_to.keys():
            self.var_pts_to[x].pop(v)
        # next
        self.next.pop(v)
        for v1 in self.heap_items:
            self.next[v1].pop(v)
        # reachable
        for x in self.var_pts_to.keys():
            self.reachable[x].pop(v)
        # cycle
        self.in_cycle.pop(v)
        # is_summary
        self.is_summary.pop(v)
        # is_shared
        self.is_shared.pop(v)        

    def set_var(self, var, new_address):
        """
        Sets var to point to node instead of its previous location in the heap
        @input: var
                node new_address
        """
        current_dst = [addr for addr in self.heap_items if self.var_pts_to[var][addr] == LOGIC3_1]
        if len(current_dst) != 1:
            raise ErrorIllegalHeap
        self.var_pts_to[var][current_dst[0]] = LOGIC3_0
        # Clear variable reachable
        for v in self.heap_items:
            self.reachable[var][v] = LOGIC3_0
        # Set var to point to the new address
        self.var_pts_to[var][new_address] = LOGIC3_1
        self.reachable[var][new_address] = LOGIC3_1
        # Set reachability from 'var' to every item on the heap
        definite_reachable = self.get_all_reachable(new_address)
        maybe_reachable = self.get_all_maybe_reachable(new_address)
        for v in maybe_reachable:
            self.reachable[var][v] = LOGIC3_HALF
        for v in definite_reachable:
            self.reachable[var][v] = LOGIC3_1
        
    def focus(self,transformer):
        """
        Partial concretization method. Derives Heaps with exact predicates with regard to a specific formula
        Formula is a result of the transformer and its arguments (Compiler Design handbook,12-26):
        x:=NULL      emptyset 
        x:=t         {t(v)} 
        x:=t.n       {\exists v_1 : t(v_1)and n(v_1,v)} i.e. t points to a specific heap item and its next is definite
        x.n:=t       {x(v), t(v)}
        x:=new       emptyset
        x==NULL      {x(v)}
        x!=NULL      {x(v)}
        x==t         {x(v),t(v)}
        x!=t         {x(v),t(v)}
        Additional:
        LS x y           {x(v), y(v), }
        ODD x y          {x(v), y(v), }
        EVEN x y         {x(v), y(v), }
        LEN x y==LEN z w {x(v), y(v),z(v), w(v),  }
        @input: Transformer - the transformer to focus for
        @output: A list of Heap with 3-value logic predicates
        """
        focused_heaps = []
        if transformer.operation == TRANSFORMER_NEW:
            return [self]
        
        elif transformer.operation == TRANSFORMER_ASSIGN_VAR:
            ########################
            # x:=t   phi = {t(v)}  #
            ########################
            return self.focus_var(transformer.arg2)
        
        elif transformer.operation == TRANSFORMER_ASSIGN_NEXT:
            ###################################################
            # x:=t.n:phi = {\exists v_1 : t(v_1)and n(v_1,v)} #
            ###################################################
            
            # Focus so phi is 0 or 1 with every combination
            # t(v_1) --> if t points to a summary node, split it
            t = transformer.arg2
            focused_heaps1 = self.focus_var(t)
            # Check if t.n has more than one value. if it does, focus it
            focused_heaps2 = []
            for h1 in focused_heaps1:
                at_t =  h1.get(t)
                t_nexts = [v for v in h1.heap_items if h1.next[at_t][v] != LOGIC3_0]
                if len(t_nexts) > 0:
                    # Then must duplicate and focus each in a different case
                    # But a summary node is always pointed with 1/2. So need to split again
                    for v_1 in t_nexts:
                        if h1.is_summary[v_1] == LOGIC3_1:
                            # Split:
                            # Create new item
                            # Connect all predecessors of v_1 to new item
                            # Connect new item to v_1
                            # set t to point to new item
                            heap_split = Heap(h1.var_pts_to.keys())
                            heap_split.copy_Heap(h1)
                            new_item = heap_split.new_node()
                            heap_split.next[new_item][NULL] = LOGIC3_0
                            for v in heap_split.heap_items:
                                if heap_split.next[v][v_1] != LOGIC3_0:
                                    heap_split.next[v][new_item] = LOGIC3_1
                                heap_split.next[v][v_1] = LOGIC3_0
                            # summary node doesn't point back to what we just split
                            heap_split.next[v_1][new_item] = LOGIC3_0 
                            # new item must point with 1/2 to old item since it is a summary
                            heap_split.next[new_item][v_1] = LOGIC3_HALF
                            # Update reachable for the new item
                            # The reachability of the new item is its predecessors and3 the connecting edge
                            reaching_vars = [x for x in heap_split.var_pts_to.keys() if heap_split.reachable[x][v_1] != LOGIC3_0]
                            predecessors = [v for v in heap_split.heap_items if heap_split.next[v][new_item] != LOGIC3_0]
                            if len(predecessors) > 1:
                                definite_predecessors = [v for v in heap_split.heap_items if heap_split.next[v][new_item] == LOGIC3_1]
                                if len(definite_predecessors) > 1:
                                    heap_split.is_shared[new_item] = LOGIC3_1
                                else:
                                    heap_split.is_shared[new_item] = LOGIC3_HALF
                            for x in reaching_vars:
                                for v in predecessors:
                                    heap_split.reachable[x][new_item] = or3(heap_split.reachable[x][new_item],
                                                                      and3 (heap_split.reachable[x][v], heap_split.next[v][new_item]))
                                    # Since v_1 is a summary still, only half reachable from all the variables and only if new item is reachable
                                    heap_split.reachable[x][v_1] = and3(heap_split.reachable[x][new_item], LOGIC3_HALF)
                            # Flatten:
                            # Turn summary node to non-summary
                            # Cancel self-edge
                            # turn all incoming 'next' ptrs to definite
                            # focus over outgoing 'next' edges - not necessary, assuming singly linked lists only in the program (raise error otherwise)
                            heap_flat = Heap(h1.var_pts_to.keys())
                            heap_flat.copy_Heap(h1)
                            heap_flat.is_summary[v_1] = LOGIC3_0
                            heap_flat.in_cycle[v_1] = LOGIC3_HALF
                            heap_flat.next[v_1][v_1]  = LOGIC3_0
                            for v in heap_flat.heap_items:
                                if heap_flat.next[v][v_1] != LOGIC3_0:
                                    heap_flat.next[v][v_1] = LOGIC3_1
                            heaps_flat = []
                            v_1_nexts = [v for v in heap_flat.heap_items if heap_flat.next[v_1][v] != LOGIC3_0]
                            for dst in v_1_nexts:
                                new_heap = Heap(heap_flat.var_pts_to.keys())
                                new_heap.copy_Heap(heap_flat)
                                for disconnect in v_1_nexts:
                                    new_heap.next[v_1][disconnect] = LOGIC3_0
                                new_heap.next[v_1][dst] = LOGIC3_1
                                heaps_flat.append(new_heap)
                            focused_heaps2 +=  [heap_split]
                            focused_heaps2 += heaps_flat
            # If no edges were focused, return focused_heaps1
            if not focused_heaps2:
                return focused_heaps1
            else:
                return focused_heaps2
                        

        elif transformer.operation == TRANSFORMER_NEXT_ASSIGN:
            ################################
            # x.n:=t   phi = {x(v), t(v)}  #
            ################################
            focused_heaps = []
            focused_heaps += self.focus_var(transformer.arg1)
            focused_heaps2 = []
            for h in focused_heaps:
                focused_heaps2 += h.focus_var(transformer.arg2)
            # If did not focus anything, return self. In this case, it means heap is already focused.
            return focused_heaps2
            
        elif transformer.operation == TRANSFORMER_ASSERT or transformer.operation == TRANSFORMER_ASSUME:
            accumulated_focused_heaps = [self]
            for atomic_expr in transformer.expression.atomics:
                if atomic_expr.type == EXPR_VAREQNULL or atomic_expr.type == EXPR_VARNEQNULL:
                    ######################################
                    # x==NULL or x!=NULL   phi = {x(v)}  #
                    ######################################
                    current_focus_heaps = []
                    for heap in accumulated_focused_heaps:
                        current_focus_heaps += heap.focus_var(atomic_expr.arg1)
                    # Focused, remove self and add focused ones
                    accumulated_focused_heaps = current_focus_heaps
                    
                elif atomic_expr.type == EXPR_VAREQVAR or atomic_expr.type == EXPR_VARNEQVAR or \
                atomic_expr.type == EXPR_LS or atomic_expr.type == EXPR_ODD or atomic_expr.type == EXPR_EVEN:
                    ######################################
                    # x==t or x!= t  phi = {x(v), t(v)}  #
                    ######################################
                    current_focus_heaps1 = []
                    for heap in accumulated_focused_heaps:
                        current_focus_heaps1 += heap.focus_var(atomic_expr.arg1)
                        current_focus_heaps2 = []
                    for h in current_focus_heaps1:
                        current_focus_heaps2 += h.focus_var(atomic_expr.arg2)
                    accumulated_focused_heaps = current_focus_heaps2
                        
                elif atomic_expr.type == EXPR_LEN:
                    #######################################################
                    # LEN x,y == LEN w,z  phi = {x(v), y(v), w(v), z(v)}  #
                    #######################################################
                    current_focus_heaps1 = []
                    for heap in accumulated_focused_heaps:
                        current_focus_heaps1 += heap.focus_var(atomic_expr.arg1)
                    current_focus_heaps2 = []
                    for h1 in current_focus_heaps1:
                        current_focus_heaps2 += h1.focus_var(atomic_expr.arg2)
                    current_focus_heaps3 = []
                    for h2 in current_focus_heaps2:
                        current_focus_heaps3 += h2.focus_var(atomic_expr.arg3)
                    current_focus_heaps4 = []
                    for h3 in current_focus_heaps3:
                        current_focus_heaps4 += h3.focus_var(atomic_expr.arg4)
                    accumulated_focused_heaps = current_focus_heaps4
            return accumulated_focused_heaps
        else:
            raise ErrorParsing()
        
        
    def focus_var(self,var):
        """
        Focuses a variable predicate x(v)
        In other words, if 'var' points to a summary node, returns two heaps:
                                        heap with summary node split
                                        heap with summary node flattened
        @input: v_1 - heap address
        @output: updates 'this'
        """
        v_1 = self.get(var)
        if self.is_summary[v_1] == LOGIC3_1:
            # Split:
            # Create new item
            # Connect all predecessors of v_1 to new item
            # Connect new item to v_1
            # set t to point to new item
            heap_split = Heap(self.var_pts_to.keys())
            heap_split.copy_Heap(self)
            new_item = heap_split.new_node()
            heap_split.next[new_item][NULL] = LOGIC3_0
            for v in heap_split.heap_items:
                if heap_split.next[v][v_1] != LOGIC3_0:
                    heap_split.next[v][new_item] = LOGIC3_1
                heap_split.next[v][v_1] = LOGIC3_0
            # summary node doesn't point back to what we just split
            heap_split.next[v_1][new_item] = LOGIC3_0 
            # new item must point with 1/2 to old item since it is a summary
            heap_split.next[new_item][v_1] = LOGIC3_HALF
            # New item is reachable from everyone who could reach the old item
            reaching_vars = [x for x in heap_split.var_pts_to.keys() if heap_split.reachable[x][v_1] != LOGIC3_0]
            predecessors = [v for v in heap_split.heap_items if heap_split.next[v][new_item] != LOGIC3_0]
            for x in reaching_vars:
                # The reachability of the new item is its predecessors and3 the connecting edge
                for v in predecessors:
                    heap_split.reachable[x][new_item] = or3(heap_split.reachable[x][new_item],
                                                      and3 (heap_split.reachable[x][v], heap_split.next[v][new_item]))
            # Flatten:
            # Turn summary node to non-summary
            # Cancel self-edge
            # turn all incoming 'next' ptrs to definite
            # focus over outgoing 'next' edges - not necessary, assuming singly linked lists only in the program (raise error otherwise)
            heap_flat = Heap(self.var_pts_to.keys())
            heap_flat.copy_Heap(self)
            heap_flat.is_summary[v_1] = LOGIC3_0
            heap_flat.next[v_1][v_1]  = LOGIC3_0
            for v in heap_flat.heap_items:
                if heap_flat.next[v][v_1] != LOGIC3_0:
                    heap_flat.next[v][v_1] = LOGIC3_1
            v_1_nexts = [v for v in heap_flat.heap_items if heap_flat.next[v_1][v] != LOGIC3_0]
            if len(v_1_nexts) > 1:
                raise ErrorIllegalHeap()
            else:
                for v in v_1_nexts:
                    heap_flat.next[v_1][v] = LOGIC3_1 
            return [heap_split, heap_flat]
        else:
            return [self]
        
        
    def is_equivalent(self,other):
        """
        Checks if 'other' is logically equivalent in its heap structure and pointers to 'this'.
        Return is DEFINITE
        @input: AbstractNode other - The abstract node evaluated
        @output: bool
        """
        # Check every type of predicate for the same structure/table
        # Same program, so assumed variables are the same and ordered the same as well by the dictionary
        # Graph isomorphism is NP-hard. Must assume addresses are consistent between states, otherwise not equivalent
        
        equivalent = True
        if self.heap_items != other.heap_items:
            equivalent = False
            return equivalent
        for v in self.heap_items:
            # var_pts_to = dictionary of var_name to unary predicate (dictionary of node to 3-val bool)
            for x in self.var_pts_to.keys():
                if (self.var_pts_to[x][v] != other.var_pts_to[x][v]) or (
                    self.reachable[x][v] != other.reachable[x][v]):
                    equivalent = False
                    return equivalent
            if (self.is_shared[v] != other.is_shared[v]) or (
                self.in_cycle[v] != other.in_cycle[v]) or (
                    self.is_summary[v] != other.is_summary[v]):
                equivalent = False
                return equivalent
            for v2 in self.heap_items:
                if self.next[v][v2] != other.next[v][v2]:
                    equivalent = False
                    return equivalent
        return equivalent
        
    def get_all_reachable(self,v):
        reachables = []
        nextq = deque([v])
        visited = set([v])
        while len(nextq) > 0:
            current = nextq.popleft()
            visited.add(current)
            current_nexts = [v1 for v1 in self.heap_items if self.next[current][v1] == LOGIC3_1 and v1 != current]
            for u in current_nexts:
                if u not in visited:
                    reachables.append(u)
                    nextq.append(u)
        return reachables

    def get_all_maybe_reachable(self, v):
        """
        Returns a list of all heap items reachable from 'v'
        @input: node v
        @output: list of heap items
        """
        # Implements BFS - "maybe next" is good enough

        reachables = []
        nextq = deque([v])
        visited = set([v])
        while len(nextq) > 0:
            current = nextq.popleft()
            visited.add(current)
            current_nexts = [v1 for v1 in self.heap_items if self.next[current][v1] != LOGIC3_0 and v1 != current and v1 not in visited]
            for u in current_nexts:
                reachables.append(u)
                nextq.append(u)         
        return reachables

    def check_reachable(self,src, dst):
        """
        Checks dst is reachable from src using "next"
        @input: node src
                node dst
        @output: 3-val bool
        """
        # Implements a BFS - two queues, "definite_next" and "maybe_next". 
        # If "definite_next" reaches dst, return 1
        # otherwise, if "maybe_next" reaches dst, return 1/2
        # else return 0
        path = self.path(src,dst)
        if path == -1:
            return LOGIC3_0
        half_edges = [e[0] for e in path if e[0] == LOGIC3_HALF]
        if half_edges:
            return LOGIC3_HALF
        else:
            return LOGIC3_1
        
        
    def check_shared(self, dst):
        """
        Checks whether dst is the 'next' of more than 1 item
        @input: node dst
        @output: 3-val bool
        """
        # Get all heap_items whose definite next is dst
        definite_dst_preds = []
        for v in self.heap_items:
            if self.next[v][dst] == LOGIC3_1:
                definite_dst_preds.append(v)
        if len(definite_dst_preds) > 1:
            return LOGIC3_1
        maybe_dst_preds = []
        for v in self.heap_items:
            if self.next[v][dst] == LOGIC3_HALF:
                maybe_dst_preds.append(v)
        if len(maybe_dst_preds) > 1:
            return LOGIC3_HALF
        return LOGIC3_0

    def check_cycle(self,v):
        """
        Checks whether 'v' is part of a cycle
        Uses check_reachable
        @input: node dst
        @output: 3-val bool
        """
        if self.is_summary[v] == LOGIC3_1:
            return LOGIC3_HALF
        v_next = [u for u in self.heap_items if self.next[v][u] != LOGIC3_0]
        for u in v_next:
            cycle = self.check_reachable(u,v)
            if cycle != LOGIC3_0:
                return cycle
        return LOGIC3_0
    
    def path(self,src, dst, definite = 0):
        """
        Returns the path from src to dst, if one exists, and appends it to current path
        Runs BFS to find the shortest path
        If no path exists, returns -1
        @input: node src
                node dst
                definite - bool whether 1/2 path is acceptable or not.
        @output: path - list of 2-tuples (edge type = 1 or 1/2, dst). If no path exists, returns -1
        """
        if definite == 1:
            if src == dst:
                return [(1, src)]
            else:
                visited = set([src])
                to_visit = deque([[(1,src)]])
                while to_visit:
                    path = to_visit.popleft()
                    node = path[-1][1] # Get last node in path
                    visited.add(node)
                    if node == dst:
                        return path
                    node_next = [v for v in self.heap_items if self.next[node][v] == LOGIC3_1 and v not in visited]
                    for v in node_next:
                        new_path = list(path)
                        new_path.append((1,v))
                        to_visit.append(new_path)
        else:
            if src == dst:
                return [(1, src)]
            else:
                visited = set([src])
                to_visit = deque([[(1,src)]])
                while to_visit:
                    path = to_visit.popleft()
                    node = path[-1][1] # Get last node in path
                    visited.add(node)
                    if node == dst:
                        return path
                    node_next = [v for v in self.heap_items if self.next[node][v] != LOGIC3_0 and v not in visited]
                    for v in node_next:
                        new_path = list(path)
                        new_path.append((self.next[node][v],v))
                        to_visit.append(new_path)
        # If did not return yet, no path found
        return -1                    
                        
                    
                
    
    def rename(self):
        """
        Renames the IDs in the heap to include 1,2...n without gaps.
        Used to limit the amount of options in the disjunctive completion
        @input: 
        @output: 
        """
        min_free_id = 1
        max_id = None
        while (min_free_id != max_id):
            # Find first free id
            ids = [v for v in self.heap_items if v != NULL]
            max_id = max(ids)
            # No need for max_id + 1 - since its the max id, it is surely taken
            for i in range(1, max_id+1):
                min_free_id = i
                if i not in ids:
                    break
            if min_free_id != max_id:
                # Set the max_id item's id to min_free_id
                self.rename_item(max_id, min_free_id)
                min_free_id = 1
                self.max_heap_index = max_id
            
    def rename_item (self, current_id, new_id):
        """
        Renames the current_id item to the new id
        @input: node current_id
                node new id
        @output:  edits 'this'
        """
        # Remove current_id from heap_items
        self.heap_items.remove(current_id)
        self.heap_items.append(new_id)
        # Go over every type of predicate, rename keys
        # var_pts_to & reachable
        for x in self.var_pts_to.keys():
            self.var_pts_to[x][new_id] = self.var_pts_to[x][current_id]
            del self.var_pts_to[x][current_id]
            # Every variable that reaches current id must reach new id
            self.reachable[x][new_id] = self.reachable[x][current_id]
            del self.reachable[x][current_id] 
        # is summary
        self.is_summary[new_id] = self.is_summary[current_id]
        del self.is_summary[current_id]
        # in cycle
        self.in_cycle[new_id] = self.in_cycle[current_id]
        del self.in_cycle[current_id]
        # next
        # Make new id's next those of the current id
        self.next[new_id] = deepcopy(self.next[current_id])
        for v in self.heap_items:
            # Make the new item the next of anyone whose next is current_id. Remove mention of current id from all heap_item's nexts ( 0 or 1)
            self.next[v][new_id] = self.next[v][current_id]
            del self.next[v][current_id]
        del self.next[current_id] 
        
        # is shared
        self.is_shared[new_id] = self.is_shared[current_id]
        del self.is_shared[current_id]
        
        

        
    
    
    
    
    
class Transformer:
    """
    Class for a transformer
    Fields:
        * string operation
        * Expression expression
        * string arg1
        * string arg2
        * string - entire command
    """
    
    def __init__(self, command_string):
        self.entire_command = command_string
        assign_pos = command_string.find(":=")
        # If some type of assign
        if assign_pos != -1:
            self.arg1 = command_string[:assign_pos]
            self.arg2 = command_string[assign_pos+2:]
            n_pos_arg1 = self.arg1.find(".n")
            n_pos_arg2 = self.arg2.find(".n")
            if self.arg2 == "new":
                self.operation = TRANSFORMER_NEW
            elif n_pos_arg1 != -1:
                self.operation = TRANSFORMER_NEXT_ASSIGN
                self.arg1 = self.arg1[:-2]
                
            elif n_pos_arg2 != -1:
                self.operation = TRANSFORMER_ASSIGN_NEXT
                self.arg2 = self.arg2[:-2]
            # Else its a regular var assignment
            else:
                self.operation = TRANSFORMER_ASSIGN_VAR
        # Assume or assert command
        elif command_string.find("assume") != -1:
            self.expression = Expression(command_string[command_string.find("assume") + 6:], False)
            self.operation = TRANSFORMER_ASSUME
        elif command_string.find("assert") != -1:
            self.expression= Expression(command_string[command_string.find("assert") + 6:], True)
            self.operation = TRANSFORMER_ASSERT
        else:
            raise ErrorParsing()
   

class AtomicExpression:
    """
    Class for a single evaluatable command condition ('b' in the documentation)
    Fields:
        * type
        * string - arg1
        * string - arg2
        * string - arg3 = only for LEN
        * string - arg4 = only for LEN
        
    Types (for field Type):
        EXPR_VAREQNULL = "var==NULL"
        EXPR_VARNEQNULL = "var!=NULL"
        EXPR_VAREQVAR = "var==var"
        EXPR_VARNEQVAR = "var!=var"
        EXPR_LS = "ls x y"
        EXPR_ODD = "odd"
        EXPR_EVEN = "even"
        EXPR_LEN = "len x y == len z w"
    """
      
    def __init__(self, expr_string):
        """
        Constructor for a single logical expression
        @input: string expr_string containing the expression
        """
        eq_pos = expr_string.find("==")
        len_pos = expr_string.find("LEN")
        split = expr_string.split()
        if eq_pos != -1 and len_pos == -1:
            self.arg2 = expr_string[eq_pos+2:]
            self.arg1 = expr_string[:eq_pos]
            if self.arg2 == "NULL":
                self.type = EXPR_VAREQNULL
            else:
                if self.arg2.find(".n") != -1:
                    self.arg2 = self.arg2[:-2]
                    self.type = EXPR_VAREQVARNEXT
                else:
                    self.type = EXPR_VAREQVAR
            
        neq_pos = expr_string.find("!=")
        if neq_pos != -1:
            self.arg2 = expr_string[neq_pos+2:]
            self.arg1 = expr_string[:neq_pos]
            if self.arg2 == "NULL":
                self.type = EXPR_VARNEQNULL
            else:
                if self.arg2.find(".n") != -1:
                    self.arg2 = self.arg2[:-2]
                    self.type = EXPR_VARNEQVARNEXT
                else:
                    self.type = EXPR_VARNEQVAR
                
        ls_pos = expr_string.find("LS")
        if ls_pos != -1:
            self.arg1 = split[1]
            self.arg2 = split[2]
            self.type = EXPR_LS
            
            
        odd_pos = expr_string.find("ODD")
        if odd_pos != -1:
            self.arg1 = split[1]
            self.arg2 = split[2]
            self.type = EXPR_ODD         
            
        even_pos = expr_string.find("EVEN")
        if even_pos != -1:
            self.arg1 = split[1]
            self.arg2 = split[2]
            self.type = EXPR_EVEN
            
        if len_pos != -1:
            self.type = EXPR_LEN
            self.arg1 = split[1]
            self.arg2 = split[2]
            self.arg3 = split[5]
            self.arg4 = split[6]
        
        if expr_string == EXPR_TRUE:
            self.type = EXPR_TRUE
        elif expr_string == EXPR_FALSE:
            self.type = EXPR_FALSE
            
    def evaluate(self, heap):
        """
        evaluate the expression, returning a 3-val bool.
        Assumed that focus for this expression was already made before the call.
        @input: heap - the Heap in which we want to evaluate the expression
        @output: 
        """
        ans = LOGIC3_HALF
        if self.type == EXPR_EVEN:
            src = self.arg1
            dst = self.arg2
            path = heap.path(heap.get(src), heap.get(dst))
            if path == -1:
                ans = LOGIC3_0
            half_edges = [e[0] for e in path]
            if not half_edges:
                if len(path) % 2 == 0:
                    ans = LOGIC3_1
                else:
                    ans = LOGIC3_0
            else:
                ans = LOGIC3_HALF
                
        elif self.type == EXPR_ODD:
            src = self.arg1
            dst = self.arg2
            path = heap.path(heap.get(src), heap.get(dst))
            if path == -1:
                ans = LOGIC3_0
            half_edges = [e[0] for e in path if e[0] == LOGIC3_HALF]
            if not half_edges:
                if len(path) % 2 == 1:
                    ans = LOGIC3_1
                else:
                    ans = LOGIC3_0
            else:
                ans = LOGIC3_HALF
            
        elif self.type == EXPR_LEN:
            src1 = self.arg1
            dst1 = self.arg2
            src2 = self.arg3
            dst2 = self.arg4
            # Paths are lists of 2-tuples (1 or 1/2, heap item)
            path1 = heap.path( heap.get(src1), heap.get(dst1))
            path2 = heap.path(heap.get(src2), heap.get(dst2))
            if path1 == -1 or path2 == -1:
                ans =  LOGIC3_0
            else:
                path1_items = [x[1] for x in path1]
                path2_items = [x[1] for x in path2]
                summaries1 = [v for v in heap.heap_items if heap.is_summary[v] == LOGIC3_1 and v in path1_items]
                summaries2 = [v for v in heap.heap_items if heap.is_summary[v] == LOGIC3_1 and v in path2_items]
                # If path1 and path2 have no summaries, calculate:
                if len(summaries1) == 0 and len(summaries2) == 0:
                    if len(path1) == len(path2):
                        ans = LOGIC3_1
                    else:
                        ans =  LOGIC3_0
                # If path1 and path2 have same summary nodes, sum up the rest
                if set(summaries1) == set(summaries2):
                    remainder1 = [v for v in path1_items if v not in summaries1]
                    remainder2 = [v for v in path2_items if v not in summaries2]
                    if len(remainder1) == len(remainder2):
                        ans = LOGIC3_1
                    else:
                        ans = LOGIC3_0
                # Otherwise they have different summaries and we have no way to know
                else:
                    ans = LOGIC3_HALF
                
        
        elif self.type == EXPR_LS:
            dsts = [v for v in heap.var_pts_to[self.arg2].keys() if heap.var_pts_to[self.arg2][v] != LOGIC3_0]
            srcs = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            if len(srcs) > 1 or len(dsts) > 1:
                ans = LOGIC3_HALF
            elif NULL in srcs or NULL in dsts:
                raise ErrorNullDereference()
            elif len(srcs) == 0 or len(dsts) == 0:
                raise ErrorParsing()
            else:
                ans = heap.reachable[self.arg1][dsts[0]]


            
        elif self.type == EXPR_VAREQNULL:
            at_var1 = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            if len(at_var1) > 1:
                ans = LOGIC3_HALF
            elif len(at_var1) == 0:
                raise ErrorParsing()
            elif NULL in at_var1:
                return LOGIC3_1
            else:
                return LOGIC3_0
            
        elif self.type == EXPR_VAREQVAR:
            at_var2 = [v for v in heap.var_pts_to[self.arg2].keys() if heap.var_pts_to[self.arg2][v] != LOGIC3_0]
            at_var1 = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            if len(at_var1) > 1 or len(at_var2) > 1:
                ans = LOGIC3_HALF
            elif len(at_var1) == 0 or len(at_var2) == 0:
                raise ErrorParsing()
            # No NULL dereference danger. if x= NULL and y = NULL, x==y is TRUE
            elif heap.var_pts_to[self.arg2][at_var2[0]] == LOGIC3_1 and heap.var_pts_to[self.arg1][at_var1[0]] == LOGIC3_1:
                if at_var2[0] == at_var1[0]:
                    ans = LOGIC3_1
                else:
                    ans = LOGIC3_0
            else:
                ans = LOGIC3_HALF
                
        elif self.type == EXPR_VAREQVARNEXT:
            at_var2 = [v for v in heap.var_pts_to[self.arg2].keys() if heap.var_pts_to[self.arg2][v] != LOGIC3_0]
            at_var1 = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            if len(at_var2)> 1 or len(at_var1) > 1:
                ans = LOGIC3_HALF
            elif len(at_var2) == 0 or len(at_var1) == 0:
                raise ErrorParsing
            # Only one item pointed to by y in x == y.n
            # if *y == NULL, cant go to its next...
            elif NULL in at_var2:
                raise ErrorNullDereference()
            # x and y both point to a definite item
            elif heap.var_pts_to[self.arg1][at_var1[0]] == LOGIC3_1 and heap.var_pts_to[self.arg2][at_var2[0]] == LOGIC3_1:
                # Get all items that may fit y.n
                var2_next = [v for v in heap.next[at_var2[0]].keys() if heap.next[at_var2[0]][v] != LOGIC3_0]
                if len(var2_next) == 0:
                    raise ErrorParsing()
                # More than one, or one but still don't know if next goes there - don't know
                if len(var2_next) > 1 or heap.next[at_var2[0]][var2_next[0]] == LOGIC3_HALF:
                    ans = LOGIC3_HALF
                # Else there is one and it is a definite next pointer as well. Return whether it is x
                else:
                    ans =  (at_var1[0] == var2_next[0])
            else:
                ans = LOGIC3_HALF
            
        elif self.type == EXPR_VARNEQNULL:
            # Find all items arg1 points to
            at_var1 = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            # If more than one - if null is one of them, don't know. else, arg1 != NULL
            if len(at_var1) > 1:
                if NULL in at_var1:
                    ans = LOGIC3_HALF
                else:
                    ans = LOGIC3_1
            if len(at_var1) == 0:
                raise ErrorParsing()
            # If var points to one item, if its NULL - return 0. if it isn't, return 1
            elif NULL in at_var1:
                ans = LOGIC3_0
            else:
                ans = LOGIC3_1
            
        elif self.type == EXPR_VARNEQVAR: 
            at_var2 = [v for v in heap.var_pts_to[self.arg2].keys() if heap.var_pts_to[self.arg2][v] != LOGIC3_0]
            at_var1 = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            intersection = [v for v in at_var1 if v in at_var2]
            # Both point definitely to same heap item
            if len(at_var2) == 0 or len(at_var1) == 0:
                raise ErrorParsing()
            # Couldn't possibly point to same item
            elif len(intersection) == 0:
                ans = LOGIC3_1
            elif len(intersection) > 1:
                ans = LOGIC3_HALF
            # only one item in intersection. if both definitely point to it, return 0. otherwise, don't know
            else:
                if heap.var_pts_to[self.arg1][intersection[0]] == LOGIC3_1 and heap.var_pts_to[self.arg2][intersection[0]] == LOGIC3_1:
                    ans = LOGIC3_0
                else:
                    ans = LOGIC3_HALF
                
                
                
        elif self.type == EXPR_VARNEQVARNEXT:
            # For x != y.n
            at_var1 = [v for v in heap.var_pts_to[self.arg1].keys() if heap.var_pts_to[self.arg1][v] != LOGIC3_0]
            at_var2 = [v for v in heap.var_pts_to[self.arg2].keys() if heap.var_pts_to[self.arg2][v] != LOGIC3_0]
            if len(at_var1) == 0 or len(at_var2) == 0:
                raise ErrorParsing()
            elif len(at_var1) > 1 or len(at_var2) > 1:
                ans = LOGIC3_HALF
            # Both are of length 1:
            if NULL in at_var2:
                raise ErrorNullDereference()
            # If y isn't definite
            elif heap.var_pts_to[self.arg2][at_var2[0]] == LOGIC3_HALF:
                ans = LOGIC3_HALF
            # y is definite
            else:
                var2_next = [v for v in heap.next[at_var2[0]].keys() if heap.next[at_var2[0]][v] != LOGIC3_0]
                intersection = [v for v in var2_next if v in at_var1]
                if len(intersection) == 0:
                    ans = LOGIC3_1
                elif len(intersection) > 1:
                    ans = LOGIC3_HALF
                # Intersection of length 1 - one item may-be-pointed-to by both x and y.n
                else:
                    # Same item pointed definitely
                    if heap.var_pts_to[self.arg1][intersection[0]] == LOGIC3_1 and heap.next[at_var2[0]][intersection[0]] == LOGIC3_1:
                        ans = LOGIC3_0
                    else:
                        ans = LOGIC3_HALF
            
        elif self.type == EXPR_FALSE:
            ans = LOGIC3_0
        elif self.type == EXPR_TRUE:
            ans = LOGIC3_1
        else:
            raise ErrorParsing()    
        return ans
        
class Expression:
    """
    Class for an evaluatable command condition ('b' in the documentation)
    Fields:
        * list of atomic expressions - atomics
        * bool  (0 for conjunction) - disjunction
    """
      
    def __init__(self, expr_string, disj):
        """
        Constructor for a logical expression
        @input: string expr_string containing the expression
                bool disj
        """
        # remove outer brackets
        self.atomics = []
        self.disjunction = disj
        expr_string = expr_string[1:-1]  
        expr_operator = "&"
        if disj:
            expr_operator = "|"
        # Break up to atomic expressions
        split_expr_string = expr_string.split(expr_operator)
        for expr in split_expr_string:
            self.atomics.append(AtomicExpression(expr))
    
    def evaluate(self, heap):
        """
        evaluate the expression, returning a 3-val bool.
        Assumed that focus for this expression was already made before the call.
        @input: heap - the Heap in which we want to evaluate the expression
        @output: 
        """
        ans = LOGIC3_0        
        if self.disjunction == True:
            # If disjunction, start False, go over atomic expressions and perform or3
            for a_expr in self.atomics:
                ans = or3(ans, a_expr.evaluate(heap))
            
        else:
            ans = LOGIC3_1
            for a_expr in self.atomics:
                ans = and3(ans, a_expr.evaluate(heap))
        return ans



class ErrorNullDereference(Exception):
    pass

class ErrorIllegalHeap(Exception):
    pass

class ErrorParsing(Exception):
    pass

class ErrorAssertionFailed(Exception):
    pass


def canonical_abstraction(heaps):
    """
    Canonical abstraction function.
    Derives abstract heaps from a list of heaps by using canonical abstraction
    Uses the current abstraction properties in heap.is_mergable
    Goes over every heap in heaps, merges heap items with identical abstraction properties
    Discards equivalent heaps 
    Renames items to 1,....n 
    @input: heaps - list of heaps to abstract
    @outout: heaps - abstracted set of heaps
    """
    # Rules for canonical abstraction:
    # 
    #    Merge all heap items that are the same in the 'abstraction properties' into summary nodes
    #        For our analysis - 'abstraction properties' will be var_pts_to[var], reachable[var], is_shared
    #    Predicates that are 1 in all heaps are 1 in the abstract state
    #                        0 in all heaps are 0
    #                        else they are 1/2
    # Go over all heaps. merge heap items that are identical in abstraction properties
    for heap in heaps:
        # For every item, if a different one exists with the same item, merge them.
        i = 0
        while len(heap.heap_items) > i:
            v1 = heap.heap_items[i]
            items_without_v1 = [v for v in heap.heap_items if v != v1]
            j = 0
            merged = False
            while len(items_without_v1) > j:
                v2 = items_without_v1[j]
                if heap.is_mergable(v1,v2):
                    # Merge clears v1, v2 from heap items anyway and appends the new, merged item
                    # For consistency, always merge upper id into lower id
                    heap.merge(min(v1,v2), max(v1,v2))
                    merged = True
                    break
                else:
                    j += 1
            if merged == False:
                i += 1
    # After summarizing nodes, for every heap, update heap to include only non-equivalent heaps
    # Rename first to try and catch more equivalent heaps
    for h in heaps:
        h.rename()
    heaps_out = []
    for h in heaps:
        equivalents = [heap for heap in heaps_out if h.is_equivalent(heap) or heap is h]
        if not equivalents:
            heaps_out.append(h)
        
    return heaps_out
            
def coerce_red(heaps):
    """
    Use predicates to refine each heap
    Used after a transformer was used on a group of Heap
    Derives the abstract heaps from a list of Heap by using coerce
    @input: heaps - list of Heap
    @output: list of heaps after coercion
    """  
    # every coercion property holds that if X |> Y then if X is 1, Y must be 1. if Y = 1/2, make it 1. if Y = 0, discard heap
    #                                                   if X is 0, Y must be 0. if Y = 1/2, make it 0, if Y = 1, discard heap
    # Expressions on the left hand side must be evaluated (reachable, etc...) while on the rhs its only the instrumentation predicates
    i = 0
    # increment i every iteration. If discarded heap, do not increment
    while len(heaps) > i:
        # Variable coercions:
        # \forall x in vars, x(v1) and x(v2) |> v1 = v2
        heap = heaps[i]
        items_except_null = [u for u in heap.heap_items if u != NULL]
        discard = 0
        for x in heap.var_pts_to.keys():
            x_points_to = [addr for addr in heap.heap_items if heap.var_pts_to[x][addr] != LOGIC3_0]
            # v1 = v2 is never half. only discarding is possible with this coercion
            if len (x_points_to) > 1:
                discard = 1
                break           
        if discard == 1:
            heaps.remove(heap)
            continue
            
        # exists v3: n(v3, v1) and n(v3,v2) |> v1 = v2
        for v3 in heap.heap_items:
            item_definite_nexts = [addr for addr in heap.heap_items if heap.next[v3][addr] == LOGIC3_1 ]
            j = 0
            while len(item_definite_nexts) > 1:
                v1 = item_definite_nexts[j]
                v2 =  item_definite_nexts[j+1]
                if heap.is_mergable(v1, v2) == LOGIC3_0:
                    discard = 1
                    break
                elif v1 != v2:
                    item_definite_nexts.remove(v1)
                    item_definite_nexts.remove(v2)
                    merged = heap.merge(min(v1,v2), max(v1, v2))
                    item_definite_nexts.append(merged)
            if discard == 1:
                break
        if discard == 1:
            heaps.remove(heap)
            continue
        
        # x(v) or (exists v1 : x(v1) and n*(v1,v)) |> reachable(x,v)
        for x in heap.var_pts_to.keys():
            for v in heap.heap_items:
                # phi = (exists v1 : x(v1) and n*(v1,v))
                phi = LOGIC3_0
                for v1 in heap.heap_items:
                    phi = or3(phi, and3(heap.var_pts_to[x][v1], heap.check_reachable(v1,v)))
                LHS = or3(heap.var_pts_to[x][v], phi)
                # if reachable(x,v) is indefinite, set it the evaluated LHS
                if heap.reachable[x][v] == LOGIC3_HALF:
                    heap.reachable[x][v] = LHS
                # If LHS is definite and reachable != 1/2 and != LHS
                elif LHS != LOGIC3_HALF and heap.reachable[x][v] != LHS:
                    discard = 1
                    break
            if discard == 1:
                break
        if discard == 1:
            heaps.remove(heap)
            continue
        

        # (exists v1: not is_shared(v) and v1!=v2 and n(v1,v)) |> not n(v2,v)
        # if v is not shared and is v1's next, it isn't anyone else's next
        for v in heap.heap_items:
            if v != NULL:
                for v2 in heap.heap_items:
                    phi = LOGIC3_0
                    # phi = (exists v1: not is_shared(v) and v1!=v2 and n(v1,v))
                    for v1 in heap.heap_items:
                        phi = or3(phi, and3(and3(not3(heap.is_shared[v]), v1 != v2), heap.next[v1][v]))
                    # if v isn't shared and v1 is its predecessor, it can't be that v2!=v1 points to it so discard
                    if phi == LOGIC3_1 and heap.next[v2][v] == LOGIC3_1:
                        discard = 1
                        break
                    # If phi is definite 1 and n(v2,v) isn't definite, it must be a 0
                    elif phi == LOGIC3_1:
                        heap.next[v2][v] = LOGIC3_0
                if discard == 1:
                    break
        if discard == 1:
            heaps.remove(heap)
            continue
        
        # exists v: not is_shared(v) and n(v1,v) and n(v2,v) |> v1 = v2
        for v in heap.heap_items:
            merged = 0
            for v1 in heap.heap_items:
                for v2 in heap.heap_items:
                        LHS = and3(not3(heap.check_shared(v)), and3(heap.next[v1][v], heap.next[v2][v]))
                        if LHS == LOGIC3_1:
                            # v1 = v2 is only a 2-val predicate...
                            if v1 != v2:
                                discard = 1
                                break
                            elif v1 != v2 and heap.is_mergable(v1,v2):
                                merged = 1
                                heap.merge(min (v1,v2), max(v1,v2))
                                # If merged, heap.heap_items had changed, so need to restart whole loop
                                break
                if merged == 1: break
            if merged == 1:
                continue
            if discard == 1:
                break
        if discard == 1:
            heaps.remove(heap)
            continue
        
        # n*(v1,v1) |> c(v1)
        for v in items_except_null:
            c = heap.check_cycle(v)
            if c != LOGIC3_HALF:
                if c != heap.in_cycle[v] and heap.in_cycle[v] != LOGIC3_HALF:
                    discard = 1
                    break
                else:
                    # Didn't discard because they're not contradicting - so put calculated 'c' in
                    heap.in_cycle[v] = c
        if discard == 1:
            heaps.remove(heap)
            continue
        
        # \exists v1,v2 : v1 != v2 and n(v1,v) and n(v2,v) |> is_shared(v)
        for v in items_except_null:
            predecessors = set([u for u in items_except_null if 
                                heap.next[u][v] != LOGIC3_0])
            # more than one predecessor
            if len(predecessors) > 1:
                definite_predecessors = set([u for u in items_except_null if 
                                heap.next[u][v] == LOGIC3_1])
                if len(definite_predecessors) > 1:
                    # two definite predecessors. If is_shared == 0 - discard. else, update is_shared to 1
                    if heap.is_shared[v] == LOGIC3_0:
                        discard = 1
                        break
                    else:
                        heap.is_shared[v] = LOGIC3_1
                # More than one predecessor, only 1 or less definite - maybe shared
                else:
                    heap.is_shared[v] = LOGIC3_HALF
            # Only one or less predecessor:
            elif heap.is_shared[v] == LOGIC3_1:
                discard = 1
                break
            else:
                heap.is_shared[v] = LOGIC3_0
        if discard == 1:
            heaps.remove(heap)
            continue
        
        i += 1
    return heaps
    
def heaps_join(heaps1, heaps2):
    """
    function that gives the JOIN of two list of heaps, i.e. disjunctive completion over the heaps for every state
    @input: list of heaps - heaps1
            list of heaps - heaps2
    @output: list of heaps - join(heaps1, heaps2)
    """
    # Merge two lists. get rid of duplicates.
    heaps_out = deepcopy(heaps1) 
    for h in heaps2:
        equivalents = [heap for heap in heaps_out if h.is_equivalent(heap) or heap is h]
        if not equivalents:
            heaps_out.append(h)
        
        # heaps_out = filter(lambda x:  (not h.is_equivalent(x)) or x is h, heaps_out)
    return heaps_out

def or3(arg1, arg2):
    """
    Returns the Kleene 3-value logical OR
    @input: 3-val bool arg1
            3-val bool arg2
    @output: 3-val bool
    """ 
    return max(arg1, arg2)
    
def and3(arg1, arg2):
    """
    Returns the Kleene 3-value logical AND
    @input: 3-val bool arg1
            3-val bool arg2
    @output: 3-val bool
    """ 
    return min(arg1, arg2)
    
def not3(arg):
    """
    Returns the Kleene 3-value logical NOT
    @input: 3-val bool arg1
    @output: 3-val bool
    """ 
    if arg == LOGIC3_1:
        return LOGIC3_0
    elif arg == LOGIC3_0:
        return LOGIC3_1
    else:
        return LOGIC3_HALF

def join3(arg1, arg2):
    """
    Returns the Kleene 3-value JOIN operation
    @input: 3-val bool arg1
            3-val bool arg2
    @output: 3-val bool
    """ 
    if arg1 == LOGIC3_0 and arg2 == LOGIC3_0:
        return LOGIC3_0
    elif arg1 == LOGIC3_1 and arg2 == LOGIC3_1:
        return LOGIC3_1
    else:
        return LOGIC3_HALF



    
def create_drawing(heap,index):
    """
    For use while the program is running. prints the given heap using networkx framework
    @input: heap
            index - index of sub-graph for multiple graph printing
    """
    G = nx.DiGraph()
    # Add nodes
    G.add_nodes_from(heap.heap_items, type = 'heap')
    summary_items = [v for v in heap.heap_items if heap.is_summary[v] == LOGIC3_1]
    G.add_nodes_from(summary_items, type = 'sum')
    for v in G.nodes:
        G.node[v]['label'] = v
        for x in heap.var_pts_to.keys():
            field_name = "reachable" + str(x)
            G.nodes[v][field_name] = heap.reachable[x][v]
    G.add_nodes_from([NULL], type = 'NULL')
    # Add edges using "next"
    edges_definite = []
    for v1 in heap.heap_items:
        for v2 in heap.heap_items:
            if heap.next[v1][v2] == LOGIC3_1:
                edges_definite.append((v1,v2))
    G.add_edges_from(edges_definite, type = 'definite')
    edges_maybe = []
    for v1 in heap.heap_items:
        for v2 in heap.heap_items:
            if heap.next[v1][v2] == LOGIC3_HALF:
                edges_maybe.append((v1,v2))
    G.add_edges_from(edges_maybe, type = 'maybe')
    for v in summary_items:
        G.add_edge(v,v, type= 'maybe')
    # Add variable pointers
    var_ptrs = []
    G.add_nodes_from(heap.var_pts_to.keys(), type ='vars')
    for x in heap.var_pts_to.keys():
        pointed_item = [v for v in heap.heap_items if heap.var_pts_to[x][v] == LOGIC3_1][0]
        var_ptrs.append((x,pointed_item))
    G.add_edges_from(var_ptrs, type= 'var_pts_to') 
    
    # Catalogue
    var_nodes = [(v) for (v,d) in G.nodes(data=True) if d['type'] == 'vars']
    heap_nodes = [(v) for (v,d) in G.nodes(data=True) if d['type'] == 'heap']
    summary_nodes = [(v) for (v,d) in G.nodes(data=True) if d['type'] == 'sum']
    assignment_edges = [(u,v) for (u,v,d) in G.edges(data=True) if d['type'] == 'var_pts_to']
    definite_next_edges = [(u,v) for (u,v,d) in G.edges(data=True) if d['type'] == 'definite']
    half_next_edges = [(u,v) for (u,v,d) in G.edges(data=True) if d['type'] == 'maybe']
    
    # Draw
    fig = plt.figure(index)
    pos = nx.spring_layout(G)
    nx.draw_networkx_edges(G, pos, edgelist=assignment_edges,  edge_color= 'blue',
                           arrowstyle='->', arrowsize=10, width = 3)
    nx.draw_networkx_edges(G, pos, edgelist=definite_next_edges,  edge_color= 'black',
                           arrowstyle='->', arrowsize=10, width = 3)
    nx.draw_networkx_edges(G, pos, edgelist=half_next_edges,
                           edge_color= 'black', style='dashdot', alpha= 0.5,
                           arrowsize=10, width = 3)
    nx.draw_networkx_nodes(G,pos, nodelist=summary_nodes ,
                            with_labels = True, node_size = 800,
                            node_color= 'black', alpha= 0.4)
    nx.draw_networkx_nodes(G,pos, nodelist=var_nodes ,
                            with_labels = True, node_size = 400,
                            node_color= 'blue')
    nx.draw_networkx_nodes(G,pos, nodelist=heap_nodes ,
                            with_labels = True, node_size = 500,
                            node_color= 'red')
    nx.draw_networkx_nodes(G,pos, nodelist=[NULL] ,
                            with_labels = True, node_size = 600,
                            node_color= 'red', alpha= 0.5)

    nx.draw_networkx_labels(G,pos)
    plt.axis('off')
    return fig


def draw(heap):
    create_drawing(heap,0)
    plt.show()


def draw_state(state):
    i = 0
    for heap in state.heaps:
        create_drawing(heap,i)
        i += 1
    plt.show()
    
def draw_state_to_png(state):
    i = 0
    for heap in state.heaps:
        create_drawing(heap, i)
        i+=1
        plt.savefig('ShapeAnalysisOf'  + 
          str(datetime.now().hour) + 
          str(datetime.now().minute) + 
          str(datetime.now().second) +
          state.name +
          "heap=" +
          str(i) + 
          '.png')
        cd()
    print("Heaps of state analyzed were saved in:" + 
          'ShapeAnalysisOf'  + 
          str(datetime.now().hour) + 
          str(datetime.now().minute) + 
          str(datetime.now().second) +
          state.name +
          "heap=" +
          "$heap_index"
          '.png')

    

def cd():
    plt.clf()
    plt.close()
    
def draw_cfg(cfg):
    """
    For use while the program is running. prints the given heap using networkx framework
    @input: heap
            index - index of sub-graph for multiple graph printing
    """
    G = nx.DiGraph()
    # Add nodes
    G.add_nodes_from(cfg.states, type = 'state')
    # Add edges
    edges = []
    labels = {}
    for edge in cfg.edges:
        e = (edge.src, edge.dst)
        edges.append(e)
        labels[e] = str(edge.op.entire_command)
    G.add_edges_from(edges, type = 'edge')
    
    # Catalogue
    nodes = [(v) for (v,d) in G.nodes(data=True) if d['type'] == 'state']
    edges = [(u,v) for (u,v,d) in G.edges(data=True) if d['type'] == 'edge']
    
    # Draw
    plt.figure(0)
    pos = nx.spring_layout(G, scale=300)
    nx.draw_networkx_edges(G, pos, edgelist=edges,  edge_color= 'black',
                           arrowstyle='->', arrowsize=5, width = 2)
    nx.draw_networkx_nodes(G,pos, nodelist=nodes ,
                            with_labels = True, node_size = 400,
                            node_color= 'blue', alpha= 0.8)
    nx.draw_networkx_edge_labels(G, pos, font_size=9,
                                 edge_labels=labels)
    nx.draw_networkx_labels(G,pos)
    plt.axis('off')


if __name__ == "__main__":
    main()
    
    
    