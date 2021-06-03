% Draws a graph with n vertices, having their centers at X(j),Y(j)
%
% If  E(i,j) > 0,  then an edge will be drawn between vertex i and vertex j
%
% Petter Strandmark 2009
%
function draw_graph(X,Y,E,Num)
    n = length(X);
    subplot('position',[0,0,1,1]);
    r = 0.2;
    for i = 1:length(X)
        color = 'k';
        if nargin >= 4
            if Num(i) == 0
                color = 'r'; %Source
            else
                color = 'b'; %Sink
            end
        else
            text(X(i)-0.05,Y(i),num2str(i));
        end
        draw_circle(X(i),Y(i),r,color); hold on;
    end
    
    for i = 1:n
        for j = 1:n
            if E(i,j) > 0
                p1 = [X(i);Y(i)];
                p2 = [X(j);Y(j)];
                vec = p1-p2;
                p1 = p1 - r * vec / norm(vec);
                p2 = p2 + r * vec / norm(vec);
                plot([p1(1) p2(1)],[p1(2) p2(2)],'k-'); 
                
                vec2 = [vec(2);-vec(1)];
                if vec2(1)<0
                    vec2 = -vec2;
                end
                
                d = 0.05;
                if abs(vec2(1))<eps
                    d = 0.13;
                    vec2(1)=-abs(vec2(2));
                end
                pt = (3*p1+p2)/4;
                pt = pt + d*vec2/norm(vec2);
                text(pt(1),pt(2),num2str(E(i,j)));
            end
        end
    end
    axis equal;
    axis off;
end

function draw_circle(x,y,r,str)
    th = linspace(0,2*pi,40);
    plot( x+r*cos(th), y+r*sin(th), str);
end