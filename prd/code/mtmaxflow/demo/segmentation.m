close all
clc

I = double(imread('cameraman.tif'))/255;
[m n] = size(I);

mu1 = 0.1;
mu0 = 0.7;
lambda = 0.5;

T = sparse([ (I(:)-mu1).^2 (I(:)-mu0).^2 ]);
E = edges4connected(m,m);
V = lambda*ones(length(E),1);
E = sparse(E(:,1),E(:,2),V,m*n,m*n);

[L glob time] = maxflowsingle(E,T);
L = reshape(L,size(I));

imshow(I); hold on;
contour(L,[0.5 0.5],'r-','LineWidth',2);

split = 1/2;

margin = m;
pixel_split = m*round(n*split) + 1; %Convert from [0,1] to pixels

% The overlapping pixels start at "pixel_split" and continue
% for "margin" number of pixels.
%
% "pixel_split can be a vector, if multiple splits are desired
%
[L_multi glob time_multi] = maxflowmulti(E,T,pixel_split,margin);
L_multi = reshape(L,size(I));

relative_time = time_multi / time


% Calculate the number of differences between the solutions
ndiff = sum(abs(L(:)~=L_multi(:)))
% ndiff might be nonzero due to multiple global solutions
% we can also calculate the differences in cut value
delta = cut_delta(E,T,m,n, L, L_multi)