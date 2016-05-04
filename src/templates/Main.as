package
{
	import flash.display.Sprite;

	[SWF(width="${width}", height="${height}", backgroundColor="${bgcolor}", frameRate="${fps}")]
	public class Main extends Sprite
	{
		${loop1}public var force_compile_${count}:${name};

		public function Main () {
			super ();
		}
	}
}