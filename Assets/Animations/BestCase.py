from manim import *


        
        
class GraphPropagation(Scene):
    def construct(self):
        # Create the graph
        vertices = [1, 2, 3]
        edges = [(1, 2), (2, 3)]
        
        edgeConfig = {
                "stroke_width": 3, 
                "tip_config": {
                    "tip_length": 0.15, 
                    "tip_width": 0.15
                    }
        }

        title = Title("Best Case")

        labels = "ABC"
        label_dict = {i : label for i, label in zip(vertices, labels)}
        
        v3t4 = MathTex("\Sigma (v3, t4)")
        v2t3 = MathTex("\Sigma (v2, t3)").next_to(v3t4, DOWN)
        v1t2 = MathTex("\Sigma (v1, t2)").next_to(v2t3, DOWN)

        t4 = VGroup(v3t4, v2t3, v1t2)

        v3t3 = MathTex("\leftarrow \Sigma (v3, t3)").next_to(v3t4, RIGHT)
        v2t2 = MathTex("\leftarrow \Sigma (v2, t2)").next_to(v3t3, DOWN)
        v1t1 = MathTex("\leftarrow \Sigma (v1, t1)").next_to(v2t2, DOWN)
        
        t3 = VGroup(v3t3, v2t2, v1t1)

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

        self.play(Create(title))

        self.play(Create(g), Create(t0), Create(t1), Create(t2), Create(t3), Create(t4))
        
        tex_labels = []
        labeled_vertices = []
        for v in g.vertices:
            label = MathTex(label_dict[v]).scale(0.75).next_to(g.vertices[v], UR * 0.25)
            tex_labels.append(label)
            labeled_vertices.append(VGroup(g[v], label))
            

        self.play(Create(VGroup(*tex_labels)));
        
        nCalculations = 0
        textOfPropegation = MathTex(f"n = {nCalculations}")
        textOfTick = MathTex(f"t = 0").next_to(textOfPropegation.get_center(), DOWN * 1.75)
        texts = VGroup(textOfPropegation, textOfTick).next_to(equations, LEFT * 3)

        textRectangle = SurroundingRectangle(texts, color=WHITE, buff=0.3)
        
        self.play(Create(textOfPropegation), Create(textOfTick), Create(textRectangle), Create(equationRectangle), Create(equationsRectangle))
        self.wait(1)

        first_vertex = 3
        verticesToVisit = [first_vertex]
        
        animations = []
        for t in range(6):
            animations.append(Transform(textOfTick, MathTex(f"t = {t - 1}").move_to(textOfTick.get_center())))
            animations.append(Transform(textRectangle, SurroundingRectangle(texts, color=WHITE, buff=0.3)))
            
            for i in range(t):
                nCalculations = nCalculations + 1
                animations.append(Transform(textOfPropegation, MathTex(f"n = {nCalculations}").move_to(textOfPropegation.get_center())))
                animations.append(Transform(textRectangle, SurroundingRectangle(texts, color=WHITE, buff=0.3)))
                verticesToVisit = [first_vertex]

                animations.append(Transform(equationsRectangle, SurroundingRectangle(equationList[i], color=RED, buff=0.1)))
                animations.append(Transform(equationRectangle, SurroundingRectangle(equationList[i][0], color=YELLOW, buff=0.1)))
                
                animations.append(g[first_vertex].animate.set_color(YELLOW))
                animations.append(FocusOn(g[first_vertex], run_time = 0.75))

                self.play(AnimationGroup(*animations))
                
                for j in range(min(i, 3)):
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
                                animations.append(ShowPassingFlash(Line(g[edge[1]], g[edge[0]]).copy().set_color(RED), time_width=2.0, line_width=10, run_time = 1))
                                animations.append(g[edge[0]].animate.set_color(RED))
                                animations.append(FocusOn(g[edge[0]], run_time = 0.75))
                
                    if edgeHit:
                        nCalculations = nCalculations + 1
                        animations.append(Transform(textOfPropegation, MathTex(f"n = {nCalculations}").move_to(textOfPropegation.get_center())))
                        animations.append(Transform(equationRectangle, SurroundingRectangle(equationList[i][(j+1)%3], color=YELLOW, buff=0.1)))
                    
                    else:
                        animations.clear()
                        break
                    
                    if animations:
                        self.play(AnimationGroup(*animations))
                    
                    animations.clear()
                    
                    verticesToVisit = newVerticesToVisit

        
        self.wait(2)

