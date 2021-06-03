close all;
clear all;
clc
set(0,'DefaultFigureWindowStyle','docked');


%Dimensions
nX = 5;
nY = 5;
nZ = 3;

%Positions of the vertices
[X Y Z] = ndgrid(1:nX,1:nY,1:nZ);

%Generate connectivity indices
E = edges3D( @edges8connected, nX,nY,nZ);

%Sparse matrix where E(i,j) > 0 iff there is an edge between i and j
E = sparse(E(:,1),E(:,2),1);

%Draw the graph
Labels = rand(nX,nY,nZ) > 0.5;
draw_graph3D(X(:),Y(:),Z(:),E,Labels);