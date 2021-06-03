function [I map] = mrgb2ind(X,varargin)
%	[I map] = mrgb2ind(X,n) / [I map] = mrgb2ind(X,map)
%	Input: X [M N L] -- color / grayscalse image
%		   n -- desired number of colors
%		   map -- desired color map
%	Output: I [M x N] int32 -- indexed image. It is 1-based.
%			map [n x 3] double -- color map
%

if(nargin<1)
	error('mrgb2ind(X,n) or mrgb2ind(X,map)');
end
if(length(varargin{1})==1)
	n_colors = varargin{1};
	% handle grayscalse images
	if size(X,3)==1
		m1 = min(X(:));
		m2 = max(X(:));
		X = (X-m1)/(m2-m1);		
		[I map] = gray2ind(X,n_colors);
		I = int32(I)+1;
		map = map*(m2-m1)+m1;
	else
		m1 = min(X(:));
		m2 = max(X(:));
		X = (X-m1)/(m2-m1);
		[I map] = rgb2ind(X,n_colors,'nodither');
		I = int32(I)+1;
		map = map*(m2-m1)+m1;
	end
	% if the image was unimodal sort the colors by their brightness value
	[ans i] = sort(sum(abs(map),2),'ascend');
	p = [];
	p(i) = 1:size(map,1);
	I = p(I);
	map = map(i,:);
	I = int32(I);
else
	map = varargin{1};
	%I = rgb2ind(gray2rgb(X),map);
	if size(X,3)==1
		I = gray2ind(X,size(map,1));
		I = int32(I+1);
	else
		I = rgb2ind(X,map,'nodither');
		I = double(I+1);
	end
end
end