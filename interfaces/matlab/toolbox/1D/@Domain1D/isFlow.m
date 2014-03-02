function a = isFlow(d)
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :return:
%     1 if the domain is a flow domain, and 0 otherwise.
%

t = domainType(d);
if t == 50
    a = 1;
else
    a = 0;
end
