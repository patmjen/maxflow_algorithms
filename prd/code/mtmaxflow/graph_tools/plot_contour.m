%PLOT_CONTOUR plots a contour in a printable way
function plot_contour(I,style)
	if nargin < 2
		style = '-';
	end
	contour(I,[0.5 0.5],style,'LineWidth',5,'Color',[1 0 0]);
	contour(I,[0.5 0.5],style,'LineWidth',2,'Color',[0.99 0.99 0.99]);
end