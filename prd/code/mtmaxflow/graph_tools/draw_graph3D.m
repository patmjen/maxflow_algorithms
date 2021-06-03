% Draws a graph with n vertices, having their centers at X(j),Y(j),Z(j)
%
% If  E(i,j) > 0,  then an edge will be drawn between vertex i and vertex j
%
% Petter Strandmark 2009
%
function draw_graph3D(X,Y,Z,E,Num)
    n = length(X);
    
    r = 0.14;
    for i = 1:length(X)
        color = [1 1 1];
        if nargin >= 5
            if Num(i) == 0
                color = [0 0 0]; %Source
            end
        end
        draw_circle(X(i),Y(i),Z(i),r,color); hold on;
    end
    
    for i = 1:n
        for j = 1:n
            if E(i,j) > 0
                p1 = [X(i);Y(i);Z(i)];
                p2 = [X(j);Y(j);Z(j)];
                vec = p1-p2;
                p1 = p1 - r * vec / norm(vec);
                p2 = p2 + r * vec / norm(vec);
                plot3([p1(1) p2(1)],[p1(2) p2(2)],[p1(3) p2(3)],'k-','LineWidth',2); 
                
            end
        end
    end
    axis equal;
    axis off;
end

function draw_circle(x,y,z,r,color)
    [X Y Z] = sphere(10);
    X = r*X + x;
    Y = r*Y + y;
    Z = r*Z + z;
    mesh(X,Y,Z,ones(size(Z)),'EdgeColor',[0 0 0],'FaceColor',color);
end