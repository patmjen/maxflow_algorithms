%
% Generates a 8-connectivity structure for a height*width grid
%
% if only_one_dir==1, then there will only be one edge between node i and
% node j. Otherwise, both i-->j and i<--j will be added.
%
function E = edges8connected(height,width,only_one_dir)

if nargin < 3
    only_one_dir = 0;
end

N = height*width;
I = []; J = [];
% connect vertically (down, then up)
is = (1:N)'; is(height:height:N)=[];
js = is+1;
if ~only_one_dir
    I = [I;is;js];
    J = [J;js;is];
else
    I = [I;is];
    J = [J;js];
end
%Diagonal
js = is+1+height;
if ~only_one_dir
    I = [I;is;js];
    J = [J;js;is];
else
    I = [I;is];
    J = [J;js];
end
js = is+1-height;
if ~only_one_dir
    I = [I;is;js];
    J = [J;js;is];
else
    I = [I;is];
    J = [J;js];
end

ind = I<1;
I(ind)=[];
J(ind)=[];
ind = J<1;
I(ind)=[];
J(ind)=[];
ind = I>N;
I(ind)=[];
J(ind)=[];
ind = J>N;
I(ind)=[];
J(ind)=[];

% connect horizontally (right, then left)
is = [1:N-height]';
js = is+height;
if ~only_one_dir
    I = [I;is;js];
    J = [J;js;is];
else
    I = [I;is];
    J = [J;js];
end

E = [I,J];

end