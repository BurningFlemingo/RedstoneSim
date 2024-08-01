from manim import *
        
class WorstCase(Scene):
    def construct(self):
        # Create the graph
        vertices = [1, 2, 3]
        edges = [(1, 2), (2, 3), (3, 1)]
        
        edgeConfig = {
                "stroke_width": 3, 
                "tip_config": {
                    "tip_length": 0.15, 
                    "tip_width": 0.15
                    }
        }

        title = Title("Worst Case")

        labels = "ABC"
        label_dict = {i : label for i, label in zip(vertices, labels)}
        
        v3t4 = MathTex("\Sigma (v3, t4)")
        v2t3 = MathTex("\Sigma (v2, t3)").next_to(v3t4, DOWN)
        v1t2 = MathTex("\Sigma (v1, t2)").next_to(v2t3, DOWN)
        v3t1r = MathTex("\Sigma (v3, t1)").next_to(v1t2, DOWN)
        v2t0r = MathTex("\Sigma (v2, t0)").next_to(v3t1r, DOWN)

        t4 = VGroup(v3t4, v2t3, v1t2, v3t1r, v2t0r)

        v3t3 = MathTex("\leftarrow \Sigma (v3, t3)").next_to(v3t4, RIGHT)
        v2t2 = MathTex("\leftarrow \Sigma (v2, t2)").next_to(v3t3, DOWN)
        v1t1 = MathTex("\leftarrow \Sigma (v1, t1)").next_to(v2t2, DOWN)
        v3t0r = MathTex("\leftarrow \Sigma (v3, t0)").next_to(v1t1, DOWN)
        
        t3 = VGroup(v3t3, v2t2, v1t1, v3t0r)

        v3t2 = MathTex("\leftarrow \Sigma (v3, t2)").next_to(v3t3, RIGHT)
        v2t1 = MathTex("\leftarrow \Sigma (v2, t1)").next_to(v3t2, DOWN)
        v1t0 = MathTex("\leftarrow \Sigma (v1, t0)").next_to(v2t1, DOWN)
        
        t2 = VGroup(v3t2, v2t1, v1t0)

        v3t1 = MathTex("\leftarrow \Sigma (v3, t1)").next_to(v3t2, RIGHT)
        v2t0 = MathTex("\leftarrow \Sigma (v2, t0)").next_to(v3t1, DOWN)
        
        t1 = VGroup(v3t1, v2t0)
        
        t0 = MathTex("\leftarrow \Sigma (v3, t0)").next_to(v3t1, RIGHT)

        equationList = [t0, t1, t2, t3, t4]

        equations = VGroup(t0, t1, t2, t3, t4)
        equations.scale(0.75).next_to(title, DOWN).to_edge(RIGHT, buff=1)

        equationsRectangle = SurroundingRectangle(t0, color=RED, buff=0.1)
        equationRectangle = SurroundingRectangle(t0[0], color=RED, buff=0.1)

        ogGraph = DiGraph(vertices, edges, layout = "circular", edge_config = edgeConfig, vertex_config = {"radius": 0.10}).scale(1).to_edge(DOWN, buff=1)
        g = ogGraph.copy()

        self.add(title)

        self.add(g, t0, t1, t2, t3, t4)
        
        tex_labels = []
        labeled_vertices = []
        for v in g.vertices:
            label = MathTex(label_dict[v]).scale(0.75).next_to(g.vertices[v], UR * 0.25)
            tex_labels.append(label)
            labeled_vertices.append(VGroup(g[v], label))
            

        self.add(VGroup(*tex_labels));
        
        nCalculations = 0
        textOfPropegation = MathTex(f"n = {nCalculations}")
        textOfTick = MathTex(f"t = 0").next_to(textOfPropegation.get_center(), DOWN * 1.75)
        texts = VGroup(textOfPropegation, textOfTick).next_to(equations, LEFT * 3)

        textRectangle = SurroundingRectangle(texts, color=WHITE, buff=0.3)
        
        self.add(textOfPropegation, textOfTick, textRectangle, equationRectangle, equationsRectangle)

