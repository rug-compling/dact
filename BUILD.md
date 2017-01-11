## Ubuntu on WSL

~~~
gbp buildpackage --git-upstream-branch=master --git-debian-branch=ubuntu/xenial \
  --git-upstream-tree=branch  --git-builder="debuild -i -I -rfakeroot-tcp"
~~~
