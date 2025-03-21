RPMSORT(1)

# NAME

rpmsort - Sort input by RPM Package Manager (RPM) versioning

# SYNOPSIS

*rpmsort* _FILE_ ...

# DESCRIPTION

*rpmsort*(1) sorts the input files, and writes a sorted list to standard
out - like *sort*(1), but aware of RPM versioning.

If '-' is given as an argument, or no arguments are given, versions are read
from standard in and written to standard out.

# EXAMPLES

```
$ echo -e 'rpm-4.18.0-3.fc38.x86_64\\nrpm-4.18.0-1.fc38.x86_64' | rpmsort 
rpm-4.18.0-1.fc38.x86_64 
rpm-4.18.0-3.fc38.x86_64
```
