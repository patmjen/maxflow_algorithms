function mmontage(A)

if(iscell(A))
sz = size(A{1});
lA = length(A);
A = reshape(cell2mat(A),[sz lA]);
end

sz = size(A);

if(length(sz)<3)
	sz(3) = 1;
end

N = sz(end);
n1 = ceil(sqrt(N));
n2 = floor((N-1)/n1)+1;

%global scaling
%a_max = max(A(A(:)<inf));
%a_min = min(A(A(:)>-inf));

for i=1:N
	subplot(n1,n2,i);
	if ndims(A)==3
		a = A(:,:,i);
	end
	if ndims(A)==4
		a = A(:,:,:,i);
	end	
	%local scaling
	a_max = max(a(a(:)<inf));
	a_min = min(a(a(:)>-inf));
	
	%if(a_max-a_min>0)		
	%end
	c = 0;%(a_min+a_max)/2;
	%d = (a_max-a_min)*1.5;
	d = max(a_max,-a_min);
	if(any(isinf(A(:))))
		d = d*1.5;
	end
	
	if(a_max-a_min>0)
		%a(a==inf) = d/2;
		%a(a==-inf) = -d/2;
		imagesc(a,[c-d c+d]);%colorbar;
	else
		imagesc(a);
	end
	%axis off;
	set(gca,'XMinorTick','off','YMinorTick','off');
	set(gca,'XTick',[],'YTick',[]);
end

end