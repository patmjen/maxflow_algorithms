%
% Creates a 2D grid and displays it
%

clear all;
clc
set(0,'DefaultFigureWindowStyle','docked');

%Dimensions
M=7;
N=9;

%Positions of the vertices
[X Y] = meshgrid(1:N,1:M);

%Convert to one-dimensional arrays
X = X(:);
Y = Y(:);
n = length(X);

%Generate connectivity indices
% A = edges4connected(M,N,1);
A = edges8connected(M,N,1);

%Edge weights
V = round(10*rand(1,size(A,1))+1);

%Sparse matrix where E(i,j) > 0 iff there is an edge between i and j
E = sparse(A(:,1),A(:,2),V,n,n,8*n);

%Draw the graph
figure(1); clf;
draw_graph(X,Y, E, ones(1,n));