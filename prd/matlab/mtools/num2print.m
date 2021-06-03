function s = num2print(a)
	s = num2str(a);
	s(:,end+1) = sprintf('\n');
	s = s';
end