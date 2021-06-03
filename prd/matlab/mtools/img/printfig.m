function printfig(out)

%axis off; box off;
%ffit;

%{
set(gca,'Units','points');
p = get(gca,'Position');
r = get(gca,'PlotBoxAspectRatio');
r = r(1)/r(2);
if p(4)<p(3)
	psz = [p(4)*r p(4)];
else
	psz = [p(3) p(3)/r];
end
isz = psz;
set(gca,'Units','normalized');

set(gcf, 'PaperUnits', 'points');
set(gcf,'PaperPosition',[10 10 isz]);
%}

%print(gcf,'-djpeg','-r0 -noui -zbuffer', out);
rgb = imcapture(gcf,'img',150);
imwrite(rgb,out,'Quality',100);

end