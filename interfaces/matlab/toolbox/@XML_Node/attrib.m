function a = attrib(x, key)
% Get the XML_Node attribute with key ``key``
%
% :param x:
%     Instance of class :mat:func:`XML_Node`
% :param key:
%     String key to look up.
% :return:
%     Instance of class :mat:func:`XML_Node`
%

if nargin ~= 2 || ~isa(key,'char')
    error('Syntax error. Type "help attrib" for more information.')
end

a = ctmethods(10, 20, x.id, key);
