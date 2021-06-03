im = imread('waterfall.bmp');

m = double(rgb2gray(im));
[M,N] = size(m);

disp('building graph');

% construct graph
E = edges4connected(M,N);
V = -abs(m(E(:,1))-m(E(:,2)))+eps;
V = V - min(V) + eps;
V = 10 * V ./ max(V);
V = exp(2*V);

A = sparse(E(:,1),E(:,2),V,M*N,M*N);

% terminal weights
% connect source to leftmost column.
% connect rightmost column to target.
T = sparse([1:M;M*N-M+1:M*N]',[ones(M,1);ones(M,1)*2],ones(2*M,1)*9e9);

disp('calculating maximum flow');


[labels glob time] = maxflowsingle(A,T);

split = 1/2;
margin = M;
pixel_split = M*round(N*split) + 1; %Convert from [0,1] to pixels
[labels_d glob_d time_d] = maxflowmulti(A,T,pixel_split,margin);

time_d / time


figure(1); clf;
subplot('position',[0 0 1 1]);
imshow(im); hold on;
% contour(reshape(labels_d,[M N]),[0.5 0.5],'r','LineWidth',3);
plot_contour(reshape(labels_d,[M N]));
% plot(round(N*split)*[1 1],[0.5 M+0.5],'g:','LineWidth',3);
for y = 1:5:M
	if mod(y,2)==0
		plot(round(N*split)*[1 1],[0.5+y y+5+0.5],'-','LineWidth',5,'Color',[0.4 0.4 1]);
		plot(round(N*split)*[1 1],[0.5+y y+5+0.5],'-','LineWidth',2,'Color',[0.99 0.99 0.99]);
	end
end

