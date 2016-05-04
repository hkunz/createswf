package
{
	import flash.display.Bitmap;
	import flash.display.DisplayObject;
	import flash.display.Sprite;

	public class ExtendedSprite extends Sprite {

		public function ExtendedSprite () {
			super ();
		}

		protected function addObject (object:DisplayObject, x:int = 0, y:int = 0, alpha:Number = 1.0, visible:Boolean = true):void {
			object.x = x;
			object.y = y;
			object.alpha = alpha;
			object.visible = visible;
			addChild(object);
		}

		public function center():void {
			for (var n:int = 0; n < numChildren; ++n) {
				var obj:DisplayObject = getChildAt(n);
				obj.x = -obj.width * 0.5;
				obj.y = -obj.height * 0.5;
			}
		}
	}
}