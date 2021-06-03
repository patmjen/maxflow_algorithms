function U = im2gray(I)
switch size(I,3)
    case 3
        U = rgb2gray(I);
    case 1
        U = I;
    otherwise
        error('not defined');
end
end