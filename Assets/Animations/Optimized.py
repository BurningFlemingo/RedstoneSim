from manim import *

class Optimized(Scene):
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

        title = Title("Optimized")

        labels = "ABC"
        label_dict = {i : label for i, label in zip(vertices, labels)}

        v3t6 = MathTex("\Sigma (v3, t6)")
        v2t5 = MathTex("\Sigma (v2, t5)").next_to(v3t6, DOWN)
        v1t4 = MathTex("\Sigma (v1, t4)").next_to(v2t5, DOWN)
        v3t3r = MathTex("\Sigma (v3, t3)").next_to(v1t4, DOWN)
        v2t2r = MathTex("\Sigma (v2, t2)").next_to(v3t3r, DOWN)
        v1t1r = MathTex("\Sigma (v1, t1)").next_to(v2t2r, DOWN)
        v3t0rr = MathTex("\Sigma (v3, t0)").next_to(v1t1r, DOWN)
        
        t6 = VGroup(v3t6, v2t5, v1t4, v3t3r, v2t2r, v1t1r, v3t0rr)
        
        v3t5 = MathTex("\leftarrow \Sigma (v3, t5)").next_to(v3t6, RIGHT)
        v2t4 = MathTex("\leftarrow \Sigma (v2, t4)").next_to(v3t5, DOWN)
        v1t3 = MathTex("\leftarrow \Sigma (v1, t3)").next_to(v2t4, DOWN)
        v3t2r = MathTex("\leftarrow \Sigma (v3, t2)").next_to(v1t3, DOWN)
        v2t1r = MathTex("\leftarrow \Sigma (v2, t1)").next_to(v3t2r, DOWN)
        v1t0r = MathTex("\leftarrow \Sigma (v1, t0)").next_to(v2t1r, DOWN)
        
        t5 = VGroup(v3t5, v2t4, v1t3, v3t2r, v2t1r, v1t0r)
        
        v3t4 = MathTex("\leftarrow \Sigma (v3, t4)").next_to(v3t5, RIGHT)
        v2t3 = MathTex("\leftarrow \Sigma (v2, t3)").next_to(v3t4, DOWN)
        v1t2 = MathTex("\leftarrow \Sigma (v1, t2)").next_to(v2t3, DOWN)
        v3t1r = MathTex("\leftarrow \Sigma (v3, t1)").next_to(v1t2, DOWN)
        v2t0r = MathTex("\leftarrow \Sigma (v2, t0)").next_to(v3t1r, DOWN)

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

        equationList = [t0, t1, t2, t3, t4, t5, t6]

        equations = VGroup(t0, t1, t2, t3, t4, t5, t6)
        equations.scale(0.70).next_to(title, DOWN).to_edge(RIGHT, buff=1)

        equationsRectangle = SurroundingRectangle(t0, color=RED, buff=0.1)
        equationRectangle = SurroundingRectangle(t0[0], color=RED, buff=0.1)

        self.play(Create(title))

        dashedLine = DashedLine(start=v1t4.get_left(), end=v1t0.get_right(), dash_length=0.1, dashed_ratio=0.5).shift(DOWN * 0.25)
        self.play(Create(dashedLine), Create(t0), Create(t1), Create(t2), Create(t3), Create(t4), Create(t5), Create(t6))
        
        nCalculations = 0
        
        textOfPropegation = MathTex(f"n = {nCalculations}")
        textOfTick = MathTex(f"t = 0").next_to(textOfPropegation.get_center(), DOWN * 1.75)
        texts = VGroup(textOfPropegation, textOfTick).next_to(equations, DOWN * 3)

        textRectangle = SurroundingRectangle(texts, color=WHITE, buff=0.3)
        
        self.play(Create(textOfPropegation), Create(textOfTick), Create(textRectangle), Create(equationRectangle), Create(equationsRectangle))
        self.wait(1)

        first_vertex = 3
        verticesToVisit = [first_vertex]
        
        vertexMemoizationRectangle = SurroundingRectangle(t0, color=PINK, buff=0.1)
        cycleMemoizationRectangle = SurroundingRectangle(t0, color=PINK, buff=0.1)
        animations = []
        maxTick = 7
        memoizedTick = 0
        for outputTick in range(maxTick):
            animations.append(Transform(textOfTick, MathTex(f"t = {outputTick}").move_to(textOfTick.get_center())))
            
            memoizedVertices = []
            for cycleDepth in range(min(2, (outputTick + 1) - 2)):
                memoizedVertices.append(equationList[outputTick - cycleDepth - 2][0])
                
            memoizedGroup = VGroup(*memoizedVertices)
            if memoizedGroup:
                animations.append(Transform(cycleMemoizationRectangle, SurroundingRectangle(memoizedGroup, color=PINK, buff=0.1)))

            animations.append(Transform(vertexMemoizationRectangle, SurroundingRectangle(VGroup(*equationList[memoizedTick][:3]), color=PINK, buff=0.1)))
                
            verticesToVisit = [first_vertex]

            animations.append(Transform(equationsRectangle, SurroundingRectangle(equationList[outputTick%maxTick], color=RED, buff=0.1)))
            
            for vertexDepth in range(min(outputTick + 1, 3)):
               memoizedTick = outputTick
               
               nCalculations = nCalculations + 1
               animations.append(Transform(textOfPropegation, MathTex(f"n = {nCalculations}").move_to(textOfPropegation.get_center())))
               if len(equationList) > outputTick:
                           
                   if len(equationList[outputTick]) > vertexDepth:
                       animations.append(Transform(equationRectangle, SurroundingRectangle(equationList[outputTick][vertexDepth], color=YELLOW, buff=0.1)))
                       animations.append(Transform(textRectangle, SurroundingRectangle(texts, color=WHITE, buff=0.3)))

               if outputTick > 3 and vertexDepth == 2:
                   animations.append(Circumscribe(equationList[outputTick - 3][0]))
               
               if outputTick > 1:
                   if len(equationList[outputTick - 1]) > vertexDepth:
                       animations.append(Circumscribe(equationList[outputTick - 1][vertexDepth]))
                          
               if animations:
                   self.play(AnimationGroup(*animations), run_time = 1.1)

               
               

               
               animations.clear()
               
        self.wait(4)

