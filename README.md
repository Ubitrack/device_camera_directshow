directshow
==========
This is the directshow Ubitrack submodule.

Description
----------
The directshow contains MS Windows specific directshow framegrabber.

Usage
-----
In order to use it, you have to clone the buildenvironment, change to the ubitrack directory and add the directshow by executing:

    git submodule add https://github.com/Ubitrack/directshow.git modules/directshow


Dependencies
----------
In addition, this module has to following submodule dependencies which have to be added for successful building:

<table>
  <tr>
    <th>Component</th><th>Dependency</th>
  </tr>
  <tr>
    <td>all</td><td>utVision</td>
  </tr>
</table>

Since this component is MS Windows specific, the directshow can only be built on MS Windows.
