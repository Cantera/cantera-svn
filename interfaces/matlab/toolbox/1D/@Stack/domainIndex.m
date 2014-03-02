function n = domainIndex(s, name)
% Get the index of a domain in a stack
%
% :param s:
%     Instance of class :mat:func:`Stack`
% :param name:
%     If double, the value is returned. Otherwise,
%     the name is looked up and its index is returned.
% :return:
%     Index of domain
%

if isa(name,'double')
    n = name;
else
    n = stack_methods(s.stack_id, 109, name);
end
