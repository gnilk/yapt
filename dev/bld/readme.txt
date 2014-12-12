What: Yapt demo engine 
Vers: 2.0b
Auth: FKling
Date: 12.12.2014
---------------------------------------------------------------------------
This is a short technical description of the demo engine 'yapt' and the command line tool 'glfwplayer.exe' used to execute the scripts.

The player uses OpenGL for all rendering.
Scripts are defined in XML.

Scripts:
--------
Scripts have four main sections under the root
	- resources
	- render
	- timeline
	- signals

There are two main type of objects
	- object
	- property

Sections:
---------
Resources are executed once and before anything else. As such resource should be used for loading one-time non-changing items such as images and object.
However, any object can be a resource.

Render is the part which is executed continously. The render section contains object instances which are responsible for prouducing the output.

Timeline is one of two meachnism to control when objects are rendered.

Signals is a section which can be used to send signals to objects at specific times. Normally used as way of synchronization.

Main objects:
-------------
	- object
	- property

Object
------

For a list of possible objects and their default values see: 'plugins.html'

Object represents an instance of some functionality defined through the 'class' attribute. It can be given a name and optional a start and duration time if not using the timeline section.
	<object class="geom.PointCloud" name="data.starfield">

Objects can contain objects. And they are always rendered depth first in order of declaration.

An object can expose input variables through the 'property' XML tag. 
	<property name="numVertex">2048</property>

An object can also have a set of output properties (not exposed in the XML) which can be linked to the input properties of another object (as long as the output type matches the input type).
	<property name="vertexCount" source="data.starfield.vertexCount" />

This will cause a dependency between the input property of one object and the output property of another object. The naming is relaxed in such a way that if an object only exposes one output an input property may refer to the object directly and not the objects named output property.

	<property name="value" source="another_object" />

If an object exposes multiple output properties referring to the object alone refers directly to the first output property. In order to control this the named output property must be given.	
	<property name="value1" source="another_object.output1" />
	<property name="value2" source="another_object.output2" />


The runtime engine will first render any dependency for an object. It is therefore not necessary to declare properties in the correct rendering order. It is also perfectly legal to have forward declaration. However, take care if the dependency is not part of the same rendering tree ONLY the dependency will be rendered.

<render>
	<object class="object1" name="test">
		<object class="object2" name="test_sub1">
		</object>
	</object>

	<object class="object1" name="test2">
		<object class="object2" name="test_sub2">
			<property name="value" source="test_sub1" />
		</object>
	</object>
	<timeline>
		<execute start="0" duration="1000" object="test2" />
	</timeline>
</render>

This will render (in order) the objects in the following order:
	test2
	test_sub1
	test_sub2


Example (renders a point cloud):
--------------------------------
<yapt2>
	<resources>
		<object class="geom.PointCloud" name="data.starfield">
			<property name="numVertex">2048</property>
			<property name="range">1.0, 0.5, 1.0</property>
		</object>
	</resources>

	<render>
		<object class="gl.RenderContext" name="mycontext">
			<property name="fov">35.0</property>

			<object class="gl.DrawPoints" name="render_drawstars">
				<property name="vertexCount" source="data.starfield.vertexCount" />
				<property name="vertexData" source="data.starfield.vertexData" />
			</object>
		</object>
	</render>
	<timeline>
		<execute start="0" duration="1000" object="mycontext" />
	</timeline>
</yapt2>








