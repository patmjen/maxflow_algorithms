%
% 3D grid structure
%
% Uses 2-connectivity in the z-direction
%
% height,width,depth -- dimensions of the graph
%
% edges2D -- function to generate the X-Y connectivity
%
% only_one_dir -- if only_one_dir==1, then there will only be one edge 
%                 between node i and node j. Otherwise, both i-->j and 
%                 i<--j will be added.
% 
% Petter Strandmark
%
function E = edges3D(edges2D,height,width,depth, only_one_dir, zweight)
	if nargin < 5
		only_one_dir = 0;
	end

	N = height*width*depth;
	I = []; J = []; 

	
	%The X-Y connectivity
	E2D = edges2D(height, width, only_one_dir);
	for z = 1:depth
		is = E2D(:,1) + (z-1)*height*width;
		js = E2D(:,2) + (z-1)*height*width;
		I = [I;is];
		J = [J;js];
	end

	if nargin>5
		W = ones(length(I),1);
	end
	
	% connect depth
	is = [1:N]'; 
	[X Y Z] = ndgrid(1:height,1:width,depth);
	is(sub2ind([height width depth],X(:),Y(:),Z(:))) = [];
	js = is+height*width;
	if ~only_one_dir
		I = [I;is;js];
		J = [J;js;is];
		if nargin>5
			W = [W;zweight*ones(length(is)+length(js),1)];
		end
	else
		I = [I;is];
		J = [J;js];
		if nargin>5
			W = [W;zweight*ones(length(is),1)];
		end
	end

	if nargin>5
		E = [I J W];
	else
		E = [I J];
	end
end