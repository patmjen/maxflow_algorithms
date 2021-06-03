function cfigure(h)
%
% selects the figure h as current figure for drawing
%
fs = get(0,'Children');
if ~ismember(h,fs)
	figure(h);
else
	set(0,'CurrentFigure',h);
end
%
end