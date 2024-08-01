from manim import *

class ActualPropagation(Scene):
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

        tickOfPropegation = 0
        
        textOfPropegation = MathTex(f"{tickOfPropegation}")
        self.play(Create(textOfPropegation))

        first_vertex = 6
        verticesToVisit = [first_vertex]
        self.play(
                g[first_vertex].animate.set_color(YELLOW),
                FocusOn(g[first_vertex], run_time = 1.2)
                )
        
        animations = []
        for i in range(10):
            newVerticesToVisit = []
            animations.clear()
            
            edgeHit = False
            for vertex in verticesToVisit:
                if not vertex is first_vertex:
                    animations.append(g[vertex].animate.set_color(WHITE))
                    
                for edge in edges:
                    if edge[1] == vertex: 
                        edgeHit = True
                        newVerticesToVisit.append(edge[0])
                        animations.append(ShowPassingFlash(Line(g[edge[1]], g[edge[0]]).copy().set_color(RED), time_width=2.0, line_width=10, run_time = 1.5))
                        animations.append(g[edge[0]].animate.set_color(RED))
                        animations.append(FocusOn(g[edge[0]], run_time = 1.5))
                    
        
            if edgeHit:
                tickOfPropegation = tickOfPropegation + 1
                animations.append(Transform(textOfPropegation, MathTex(f"{tickOfPropegation}")))
                
                self.play(AnimationGroup(*animations))

            else:
                if not vertex is first_vertex:
                    animations.append(g[vertex].animate.set_color(WHITE))
                break
                
            animations.clear()
            
            verticesToVisit = newVerticesToVisit
            

            if animations:
                self.play(AnimationGroup(*animations))
        
        self.wait(2)

