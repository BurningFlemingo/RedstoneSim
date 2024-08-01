from manim import *

class Depth(Scene):
    def construct(self):
        # Create the graph
        vertices = [1, 2, 3, 4, 5, 6]
        edges = [(1, 2), (1, 3), (2, 4), (2, 5), (3, 5), (3, 6), (4, 5), (5, 6), (6, 1)]
        
        edgeConfig = {
                "stroke_width": 3, 
                "tip_config": {
                    "tip_length": 0.15, 
                    "tip_width": 0.15
                    }
        }
        
        g = DiGraph(vertices, edges, layout = "circular", edge_config = edgeConfig, vertex_config = {"radius": 0.10}).scale(1.25)

        labels = "ABCDEF"
        label_dict = {i : label for i, label in zip(vertices, labels)}

        self.play(Create(Title("Depth")))
        self.play(Create(g))
        self.play(g.animate.shift(DOWN))
        
        tex_labels = []
        labeled_vertices = []
        for v in g.vertices:
            label = MathTex(label_dict[v]).scale(0.75).next_to(g.vertices[v], UR)
            tex_labels.append(label)
            labeled_vertices.append(VGroup(g[v], label))
            

        self.play(Create(VGroup(*tex_labels)));

        sizeOfPropegation = 0
        
        textOfPropegation = MathTex(sizeOfPropegation)
        self.play(Create(textOfPropegation))

        first_vertex = 6
        verticesToVisit = [first_vertex]
        self.play(
                g[first_vertex].animate.set_color(YELLOW),
                FocusOn(g[first_vertex], run_time = 1.2)
                )
        
        animations = []
        visited = []
        n = 0
        while verticesToVisit or n > 10:
            n = n+ 1
            newVerticesToVisit = []
            

            edgeHit = False
            for vertex in verticesToVisit:
                if vertex in visited:
                    continue
            
                visited.append(vertex)

                for edge in edges:
                    if edge[1] == vertex: 
                        edgeHit = True
                        newVerticesToVisit.append(edge[0])
                        animations.append(ShowPassingFlash(Line(g[edge[1]], g[edge[0]]).copy().set_color(RED), time_width=2.0, line_width=10))
                        animations.append(g.animate.remove_edges(edge))
                    
        
            if edgeHit:
                sizeOfPropegation = sizeOfPropegation + 1
                animations.append(Transform(textOfPropegation, MathTex(sizeOfPropegation)))
                
            if animations:
                self.play(AnimationGroup(*animations))
                
            animations.clear()
            
            verticesToVisit = newVerticesToVisit

            for vertex in newVerticesToVisit:
                if vertex in visited:
                    continue
                
                animations.append(g[vertex].animate.set_color(RED))
                animations.append(FocusOn(g[vertex], run_time = 1.2))

            if animations:
                self.play(AnimationGroup(*animations))
        
        self.wait(2)

