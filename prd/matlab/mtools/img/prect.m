function prect(rect,style,varargin)
%
%	prect(rect,style,varargin) % matlab rect [x y w h]
%
if isempty(varargin)
	varargin{1} = 'LineWidth';
	varargin{2} = 1;
end

k = ishold;
plot([rect(1) rect(1)+rect(3)], [rect(2) rect(2)],style,varargin{:});
hold on;
plot([rect(1) rect(1)+rect(3)], [rect(2)+rect(4) rect(2)+rect(4)],style,varargin{:});
plot([rect(1) rect(1)], [rect(2) rect(2)+rect(4)],style,varargin{:});
plot([rect(1)+rect(3) rect(1)+rect(3)], [rect(2) rect(2)+rect(4)],style,varargin{:});
if(~k)
	hold off;
end
	
end