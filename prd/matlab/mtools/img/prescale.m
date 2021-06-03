function varargout = prescale(s, varargin)
%
%   scales several images
%
    for k=1:min(length(varargin),nargout)
		if isempty(varargin{k})
			varargout(k) = {varargin{k}};
		else
			varargout(k) = {imresize(varargin{k},msize(varargin{k},[1 2])*s,'bilinear')};
		end
    end
end