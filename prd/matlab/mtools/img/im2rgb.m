function I = im2rgb(U)
switch size(U,3)
    case 1
			U = U-min(U(:));
			U = U/max(U(:));
			I = gray2rgb(U);
    case 3
      I = U;
    otherwise
      error('not defined');
end
end