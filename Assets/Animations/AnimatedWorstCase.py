from manim import *

class AnimatedWorstCase(Scene):
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

        v3t6 = MathTex("\Sigma (v3, t6)")
        v2t5 = MathTex("\Sigma (v2, t5)").next_to(v3t6, DOWN)
        v1t4 = MathTex("\Sigma (v1, t4)").next_to(v2t5, DOWN)
        v3t3r = MathTex("\Sigma (v3, t3)").next_to(v1t4, DOWN).set_color(RED)
        v2t2r = MathTex("\Sigma (v2, t2)").next_to(v3t3r, DOWN).set_color(RED)
        v1t1r = MathTex("\Sigma (v1, t1)").next_to(v2t2r, DOWN).set_color(RED)
        v3t0rr = MathTex("\Sigma (v3, t0)").next_to(v1t1r, DOWN).set_color(RED)
        
        t6 = VGroup(v3t6, v2t5, v1t4, v3t3r, v2t2r, v1t1r, v3t0rr)
        
        v3t5 = MathTex("\leftarrow \Sigma (v3, t5)").next_to(v3t6, RIGHT)
        v2t4 = MathTex("\leftarrow \Sigma (v2, t4)").next_to(v3t5, DOWN)
        v1t3 = MathTex("\leftarrow \Sigma (v1, t3)").next_to(v2t4, DOWN)
        v3t2r = MathTex("\leftarrow \Sigma (v3, t2)").next_to(v1t3, DOWN).set_color(RED)
        v2t1r = MathTex("\leftarrow \Sigma (v2, t1)").next_to(v3t2r, DOWN).set_color(RED)
        v1t0r = MathTex("\leftarrow \Sigma (v1, t0)").next_to(v2t1r, DOWN).set_color(RED)
        
        t5 = VGroup(v3t5, v2t4, v1t3, v3t2r, v2t1r, v1t0r)
        
        v3t4 = MathTex("\leftarrow \Sigma (v3, t4)").next_to(v3t5, RIGHT)
        v2t3 = MathTex("\leftarrow \Sigma (v2, t3)").next_to(v3t4, DOWN)
        v1t2 = MathTex("\leftarrow \Sigma (v1, t2)").next_to(v2t3, DOWN)
        v3t1r = MathTex("\leftarrow \Sigma (v3, t1)").next_to(v1t2, DOWN).set_color(RED)
        v2t0r = MathTex("\leftarrow \Sigma (v2, t0)").next_to(v3t1r, DOWN).set_color(RED)

        t4 = VGroup(v3t4, v2t3, v1t2, v3t1r, v2t0r)

        v3t3 = MathTex("\leftarrow \Sigma (v3, t3)").next_to(v3t4, RIGHT)
        v2t2 = MathTex("\leftarrow \Sigma (v2, t2)").next_to(v3t3, DOWN)
        v1t1 = MathTex("\leftarrow \Sigma (v1, t1)").next_to(v2t2, DOWN)
        v3t0r = MathTex("\leftarrow \Sigma (v3, t0)").next_to(v1t1, DOWN).set_color(RED)
        
        t3 = VGroup(v3t3, v2t2, v1t1, v3t0r)

        v3t2 = MathTex("\leftarrow \Sigma (v3, t2)").next_to(v3t3, RIGHT)
        v2t1 = MathTex("\leftarrow \Sigma (v2, t1)").next_to(v3t2, DOWN)
        v1t0 = MathTex("\leftarrow \Sigma (v1, t0)").next_to(v2t1, DOWN)
        
        t2 = VGroup(v3t2, v2t1, v1t0)

        v3t1 = MathTex("\leftarrow \Sigma (v3, t1)").next_to(v3t2, RIGHT)
        v2t0 = MathTex("\leftarrow \Sigma (v2, t0)").next_to(v3t1, DOWN)
        
        t1 = VGroup(v3t1, v2t0)
        
        t0 = MathTex("\leftarrow \Sigma (v3, t0)").next_to(v3t1, RIGHT)

        global equationList
        equationList = [t0, t1, t2, t3, t4, t5, t6]

        equations = VGroup(t0, t1, t2, t3, t4, t5, t6)
        equations.scale(0.70).next_to(title, DOWN).to_edge(RIGHT, buff=1)

        equationsRectangle = SurroundingRectangle(t0, color=RED, buff=0.1)
        equationRectangle = SurroundingRectangle(t0[0], color=RED, buff=0.1)

        global g
        ogGraph = DiGraph(vertices, edges, layout = "circular", edge_config = edgeConfig, vertex_config = {"radius": 0.10}).scale(1).to_edge(DR, buff=1.0)
        g = ogGraph.copy()

        self.play(Create(title))

        dashedLine = DashedLine(start=v1t4.get_left(), end=v1t0.get_right(), dash_length=0.1, dashed_ratio=0.5).shift(DOWN * 0.25)
        self.play(Create(dashedLine), Create(g), Create(t0), Create(t1), Create(t2), Create(t3), Create(t4), Create(t5), Create(t6))
        
        tex_labels = []
        labeled_vertices = []
        for v in g.vertices:
            label = MathTex(label_dict[v]).scale(0.75).next_to(g.vertices[v], UR * 0.25)
            tex_labels.append(label)
            labeled_vertices.append(VGroup(g[v], label))
            

        nCalculations = 0
        textOfPropegation = MathTex(f"n = {nCalculations}")
        textOfTick = MathTex(f"t = 0").next_to(textOfPropegation.get_center(), DOWN * 1.75)
        texts = VGroup(textOfPropegation, textOfTick).next_to(equations, DOWN * 3)

        textRectangle = SurroundingRectangle(texts, color=WHITE, buff=0.3)
        
        self.play(Create(VGroup(*tex_labels)), Create(textOfPropegation), Create(textOfTick), Create(textRectangle), Create(equationRectangle), Create(equationsRectangle))
        self.wait(1)

        
        def simulate(first_vertex, last_tick, nCalculations, textOfPropegation, textOfTick, texts, textRectangle, index):
            animations = []
            verticesToVisit = [first_vertex]

            for t in range(last_tick):
                animations.append(Transform(textOfTick, MathTex(f"t = {t - 1}").move_to(textOfTick.get_center())))
                animations.append(Transform(textRectangle, SurroundingRectangle(texts, color=WHITE, buff=0.3)))
                
                for i in range(t):
                    nCalculations = nCalculations + 1
                    animations.append(Transform(textOfPropegation, MathTex(f"n = {nCalculations}").move_to(textOfPropegation.get_center())))
                    animations.append(Transform(textRectangle, SurroundingRectangle(texts, color=WHITE, buff=0.3)))
                    verticesToVisit = [first_vertex]

                    animations.append(Transform(equationsRectangle, SurroundingRectangle(equationList[i%8], color=RED), opacity=1.0/index, buff=0.1))
                    animations.append(Transform(equationRectangle, SurroundingRectangle(equationList[i%8][0], color=YELLOW), opacity=1.0/index, buff=0.1))
                    
                    animations.append(g[first_vertex].animate.set_color(YELLOW))
                    animations.append(g[first_vertex].animate.set_opacity(1.0/index))

                    self.play(AnimationGroup(*animations), run_time = 0.20)
                    
                    for j in range(i):
                        if j >= 2 and (j+1 != i) and t - 3 > 0:
                            self.play(Transform(equationRectangle, SurroundingRectangle(equationList[i][j + 1]), color=YELLOW, opacity=1.0/index, buff=0.1, run_time=0.25))
                            nCalculations = simulate(3 - (j + 1) % 3, t - 3, nCalculations, textOfPropegation, textOfTick, texts, textRectangle, index + 1)
                        
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
                                    animations.append(ShowPassingFlash(Line(g[edge[1]], g[edge[0]]).copy().set_color(RED).set_opacity(1.0/index), time_width=2.0, line_width=10))
                                    animations.append(g[edge[0]].animate.set_color(RED))
                                    animations.append(g[edge[0]].animate.set_opacity(1.0/index))
                    
                        if edgeHit:
                            nCalculations = nCalculations + 1
                            animations.append(Transform(textOfPropegation, MathTex(f"n = {nCalculations}").move_to(textOfPropegation.get_center()), opacity=1.0/index))
                            if len(equationList) > i:
                                if len(equationList[i]) > j + 1:
                                    animations.append(Transform(equationRectangle, SurroundingRectangle(equationList[i][j + 1]), color=YELLOW, opacity=1.0/index, buff=0.1))
                        
                        else:
                            animations.clear()
                            break
                        
                        if animations:
                            self.play(AnimationGroup(*animations), run_time=0.15)
                        
                        animations.clear()
                        
                        verticesToVisit = newVerticesToVisit
                        
            return nCalculations

        simulate(3, 8, nCalculations, textOfPropegation, textOfTick, texts, textRectangle, 1)
        
        self.wait(1)

