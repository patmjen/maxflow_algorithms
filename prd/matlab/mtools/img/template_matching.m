function ssd = template_matching(template,mask,image)
%
%		template_matching(template,mask,image)	-- SSD template matching.
%
%		Old good SSD template matching: ssd(xy) = sum_{ij\in Mask}(T_{ij}-I_{ij+xy})^2.
%		
%		Input:
%				template - template image to search for;
%				mask - template mask, of the same size as template image, only pixels of 
%						template, where mask(i,j)=1 are relevant in computation;
%				image - destination image, should be larger then template;
%				!  size(image,3) == size(template,3)
%
%		Output:
%				ssd [size(image,1) x size(image,2)] - SSD values. 
%						exp(-ssd/(2*sigma^2)) is a posteriori probability 
%						distribution under assumption of independent gaussian noise;
%				
%
%		Author: Alexander Shekhovtsov
%

for i=1:size(template,3) 
	template(:,:,i) = rot90(template(:,:,i).*mask,2);
end;
mask = rot90(mask,2);

%% via direct convolution computation

X_sq = conv2(sum(image(:,:,:).^2,3),mask,'valid');
XY = zeros(size(X_sq));

for i = 1:size(image,3)
	XY = XY+conv2(image(:,:,i),template(:,:,i),'valid');
end;

Y_sq = conv2(ones(size(image,1),size(image,2)),sum(template.^2,3),'valid');

ssd = X_sq-2*XY+Y_sq;

return;



%% via FFT
SZ = size(image)+size(template)+1; SZ = SZ(1:2);

	f1 = fft2(sum(image.^2,3),SZ(1),SZ(2));
	f2 = fft2(mask,SZ(1),SZ(2));
	X_sq = real(ifft2(f1.*f2));

	XY = zeros(SZ);
	for i = 1:size(image,3)
		f1 = fft2(image(:,:,i),SZ(1),SZ(2));
		f2 = fft2(template(:,:,i),SZ(1),SZ(2));
		C = real(ifft2(f1.*f2));
		XY = XY+C;
	end;
	%Y_sq = sum(sum(sum(template(:,:,:).^2,3).*mask));
	%ssd = X_sq-2*XY+Y_sq;
	
	f1 = fft2(ones(SZ));
	f2 = fft2(sum(template.^2,3),SZ(1),SZ(2));
	Y_sq = real(ifft2(f1.*f2));

	ssd = X_sq-2*XY+Y_sq;
	%ssd = ssd(1:size(image,1),1:size(image,2),:);

end