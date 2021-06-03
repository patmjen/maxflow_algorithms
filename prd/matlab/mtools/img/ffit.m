function ffit
set(gca,'DataAspectRatio',[1 1 1]);
set(gca,'PlotBoxAspectRatioMode','auto');
set(gca,'XTickLabel',[]);set(gca,'YTickLabel',[]);
set(gca,'XTick',[]);set(gca,'YTick',[]);
set(gca,'XMinorTick','off');
set(gca,'YMinorTick','off');
set(gca,'LooseInset',[0 0 0 0]);
set(gca,'ActivePositionProperty','OuterPosition');

set(gca,'Units','pixels');
p = get(gca,'Position');
r = get(gca,'PlotBoxAspectRatio');
r = r(1)/r(2);
if p(4)<p(3)
	psz = [p(4)*r p(4)];
else
	psz = [p(3) p(3)/r];
end
ti = get(gca,'TightInset');
psz = psz+[ti(1)+ti(3) ti(2)+ti(4)];
set(gca,'Units','normalized');
set(gcf,'Units','pixels');
ssz = get(gcf,'Position');

pp = [ssz(1) ssz(2)+ssz(4)-psz(2) psz(1) psz(2)];
set(gcf,'Position',pp);
set(gcf,'Color',[1 1 1]);
set(gca,'OuterPosition',[0 0 1 1]);
end