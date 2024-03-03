// types.cs
// This file contains generated code and will be overwritten when you rerun code generation.


using System;
using System.Globalization;

namespace Altova.Types
{
	public enum DateTimeFormat
	{
		/// <summary>
		/// Format as schema dateTime.
		/// </summary>
		W3_dateTime,

		/// <summary>
		/// Format as schema date.
		/// </summary>
		W3_date,

		/// <summary>
		/// Format as schema time. 
		/// </summary>		
		W3_time,

		/// <summary>
		/// Format as schema gYear.
		/// </summary>
		W3_gYear,

		/// <summary>
		/// Format as schema gYearMonth. 
		/// </summary>
		W3_gYearMonth,

		/// <summary>
		/// Format as schema gMonth.
		/// </summary>
		W3_gMonth,

		/// <summary>
		/// Format as schema gMonthDay.
		/// </summary>
		W3_gMonthDay,

		/// <summary>
		/// Format as schema gDay.
		/// </summary>
		W3_gDay,

		/// <summary>
		/// Format as standard DateTime "YYYY-MM-DD HH:MM:SS".
		/// </summary>
		S_DateTime,

		/// <summary>
		/// Format as number of seconds since epoch.
		/// </summary>
		S_Seconds,

		/// <summary>
		/// Format as number of days since epoch.
		/// </summary>
		S_Days,

	}

	public class DateTime : System.IComparable
	{
		public const short NoTimezone = short.MinValue;

		System.DateTime myValue;
		short timezone = NoTimezone;
		
		[System.Flags]
		public enum DateTimePart
		{
			Year = 1 << 0,
			Month = 1 << 1,
			Day = 1 << 2,
			Date = Year|Month|Day,
			Hour = 1 << 3,
			Minute = 1 << 4,
			Second = 1 << 5,
			Time = Hour|Minute|Second,
			TimezoneHour = 1 << 6,
			TimezoneMinute = 1 << 7,
			Timezone = TimezoneHour|TimezoneMinute,
		}

		public override bool Equals(object obj)
		{
			if (!(obj is DateTime))
				return false;
			DateTime dt = (DateTime) obj;
			return myValue == dt.myValue && timezone == dt.timezone;
		}

		public override int GetHashCode()
		{
			return myValue.GetHashCode() ^ timezone.GetHashCode();
		}


		public int CompareTo(object obj)
		{
			if (obj == null)
				return 1;
			if (!(obj is DateTime))
				throw new ArgumentException("Argument is not of type DateTime");
			DateTime dt = (DateTime)obj;

			if ((timezone != NoTimezone && dt.timezone != NoTimezone))
			{
				return myValue.AddMinutes(-timezone).CompareTo(dt.myValue.AddMinutes(-dt.timezone));
			}
			else if (timezone != NoTimezone)
			{
				DateTime dt2 = new DateTime();
				dt2.myValue = dt.myValue;
				dt2.timezone = 14 * 60;
				return CompareTo(dt2);
			}
			else if (dt.timezone != NoTimezone)
			{
				DateTime dt2 = new DateTime();
				dt2.myValue = myValue;
				dt2.timezone = -14 * 60;
				return dt2.CompareTo(dt);
			}
			else
			{
				return myValue.CompareTo(dt.myValue);
			}
		}
	
		public class ParseContext
		{
			string source;
			int index;

			public ParseContext(string source) 
			{
				this.source = source;
				this.index = 0;
			}

			public int Index 
			{
				get { return index; }
			}

			public bool IsValid() 
			{
				return index < source.Length;
			}

			public bool Check(char expect)
			{
				if (!IsValid()) return false;
				return source[index] == expect;
			}

			public bool CheckAndAdvance(char expect)
			{
				if (!Check(expect)) return false;
				Advance();
				return true;
			}

			public void Advance()
			{
				++index;
			}

			public bool ReadDigitAndAdvance(ref int value, int scale, int maxdigit)
			{
				if (!IsValid()) return false;
				char c = source[index];
				if (c < '0' || c > '9')
					return false;
				int val = (int)c - (int)'0';
				if (val > maxdigit)
					return false;
				value += val * scale;
				Advance();
				return true;
			}
		}

		public bool ParseDateTime(string s, DateTimePart part)
		{
			return ParseDateTime(s, part, 0);
		}
		
		public bool ParseDateTime(string s, DateTimePart part, DateTimePart optional)
		{
			ParseContext context = new ParseContext(s.Trim());

			bool bDatePart = ( part & DateTimePart.Date ) != 0;
			bool bTimePart = ( part & DateTimePart.Time ) != 0;

			int year = 0, month = 0, day = 0;
			int hour = 0, minute = 0;
			double second = 0;
			
			timezone = NoTimezone;

			if ( bDatePart )
			{
				// parse date
				bool bNegative = context.CheckAndAdvance( '-' );

				if ( (part & DateTimePart.Year ) != 0 )
				{
					int digits = 0;
					int temp = 0;						
					while ( context.ReadDigitAndAdvance( ref temp, 1, 9 ) )
					{
						year = year * 10 + temp;
						digits += 1;
						temp = 0;
						if (digits >= 8) // overflow
							return false;
					}			 
					if ( digits < 4 ) // invalid.
						return false;
					if ( digits > 4 && year < 10000 )
						return false;
					if (bNegative) 
						year = -year;
				} 

				if ( (part & ( DateTimePart.Month | DateTimePart.Day )) != 0 ) 
				{
					if ( !context.CheckAndAdvance( '-' ) ) return false;

					if ( ( part & DateTimePart.Month ) != 0 )
					{
						if ( !context.ReadDigitAndAdvance( ref month, 10, 1 ) ) return false;
						if ( !context.ReadDigitAndAdvance( ref month, 1, month < 10 ? 9 : 2 ) ) return false;
						if ( month == 0 ) return false;						
					}

					if ( ( part & DateTimePart.Day ) != 0 ) 
					{
						if ( !context.CheckAndAdvance(  '-') ) return false;

						int maxFirstDigit = month != 2 ? 3 : 2;

						// complicate things by making them complicated.
						if ( !context.ReadDigitAndAdvance( ref day, 10, maxFirstDigit ) ) return false;
						if ( !context.ReadDigitAndAdvance( ref day, 1, 9 ) ) return false;
						if ( day == 0 || day > 31 ) return false;
						
						if ( ( part & DateTimePart.Month ) != 0 )
						{
							bool b1 = month <= 7;
							bool b2 = ( month & 1 ) == 0;

							// month 1, 3, 5, 7, 8, 10, 12
							if ( b1 == b2 && day > 30 )
								return false;

							// february.
							if ( month == 2 && day > 29 )
								return false;

							// leap years.
							if ( month == 2 && ( part & DateTimePart.Year ) != 0 && 
								( year % 4 != 0 || year % 100 == 0 ) && year % 400 != 0 &&
								day > 28 )
								return false;
						}
					}
				}	

				if ( bTimePart )
				{
					// a 'T' must follow
					if ( !context.CheckAndAdvance( 'T') ) return false;
				}
			}

			if ( bTimePart )
			{
				// check format here
				
				// hour from 0 to 2
				if ( !context.ReadDigitAndAdvance( ref hour, 10, 2 ) ) return false;
				if ( !context.ReadDigitAndAdvance( ref hour, 1, hour < 20 ? 9 : 4 ) ) return false;
				if ( !context.CheckAndAdvance( ':' ) ) return false;
				int maxFirstDigit = hour == 24 ? 0 : 5;
				int maxSecondDigit = hour == 24 ? 0 : 9;
				if ( !context.ReadDigitAndAdvance( ref minute, 10, maxFirstDigit ) ) return false;
				if ( !context.ReadDigitAndAdvance( ref minute, 1, maxSecondDigit ) ) return false;
				if ( !context.CheckAndAdvance( ':' ) ) return false;
				int secondInt = 0;
				if ( !context.ReadDigitAndAdvance( ref secondInt, 10, maxFirstDigit ) ) return false;
				if ( !context.ReadDigitAndAdvance( ref secondInt, 1, maxSecondDigit ) ) return false;

				second = secondInt;

				if ( context.CheckAndAdvance( '.' ) )
				{
					// fraction. do whatever seems fit.
					int val = 0;
					int digits = 0;
					while ( context.ReadDigitAndAdvance( ref val, 1, 9) )
					{
						val *= 10;
						digits += 1;
						if ( digits >= 8 ) // precision loss - ignore
							break;
					}

					if ( digits == 0 )
						return false;

					second += val * System.Math.Pow( 10.0, -digits - 1 );

					// skip any further digits.
					while ( context.ReadDigitAndAdvance( ref val, 0, 9) ) 
						;		
				}
			}

			// timezone
			if ( context.CheckAndAdvance('Z') )
			{
				// timezone specified, it is UTC.
				timezone = 0;				
			}
			else if ( context.Check('+') || context.Check('-' ) )
			{
				// timezone offset, in hour:minute format
				bool bNegative = context.Check('-');
				context.Advance();
				
				// do not check the hour part, for those who are obscure.
				int temp = 0;
				bool skipTimezoneMinute = false;
				if ( !context.ReadDigitAndAdvance( ref temp, 600, 9 ) ) return false;
				if ( !context.ReadDigitAndAdvance( ref temp, 60, 9 ) ) return false;
				if ( !context.CheckAndAdvance( ':' ) ) { if ((optional & DateTimePart.TimezoneMinute) != 0) skipTimezoneMinute = true; else return false; }
				if ( !skipTimezoneMinute) if ( !context.ReadDigitAndAdvance( ref temp, 10, 5 ) ) return false;
				if ( !skipTimezoneMinute) if ( !context.ReadDigitAndAdvance( ref temp, 1, 9 ) ) return false;

				timezone = (short)(bNegative ? -temp : temp);
			}

			if ( context.IsValid() )
				return false;

			// C# specific
			if (year <= 0) year = 1;
			if (month == 0) month = 1;
			bool badjust = false;
			if (hour == 24)
			{
				hour = 0;
				badjust = true;
			}
			if (day == 0) day = 1;
			if ((part & DateTimePart.Year) == 0 && month == 2 && day == 29)
				year = 4;
			try 
			{
				myValue = new System.DateTime(year, month, day, hour, minute, 0);
				decimal ticks = (new System.Decimal(second) * TicksPerSecond);
				myValue = myValue.AddTicks(System.Decimal.ToInt64(ticks));
				if (badjust)
					myValue = myValue.AddDays(1);
			}
			catch 
			{
				return false;
			}
			return true;

		}

		public static DateTime Parse(string s, DateTimeFormat format)
		{
			bool b = false;
			DateTime r = new DateTime();
			switch (format)
			{
				case DateTimeFormat.W3_dateTime:
					b = r.ParseDateTime(s, DateTimePart.Date|DateTimePart.Time);
					break;

				case DateTimeFormat.W3_date:
					b = r.ParseDateTime(s, DateTimePart.Date);
					break;

				case DateTimeFormat.W3_time:
					b = r.ParseDateTime(s, DateTimePart.Time);
					break;

				case DateTimeFormat.W3_gYear:
					b = r.ParseDateTime(s, DateTimePart.Year);
					break;

				case DateTimeFormat.W3_gYearMonth:
					b = r.ParseDateTime(s, DateTimePart.Year|DateTimePart.Month);
					break;

				case DateTimeFormat.W3_gMonth:
					b = r.ParseDateTime(s, DateTimePart.Month);
					break;

				case DateTimeFormat.W3_gMonthDay:
					b = r.ParseDateTime(s, DateTimePart.Month|DateTimePart.Day);
					break;

				case DateTimeFormat.W3_gDay:
					b = r.ParseDateTime(s, DateTimePart.Day);
					break;
			}
			if (!b)
				throw new StringParseException(s + " cannot be converted to a dateTime value");
			return r;
		}

		#region Constructors
		public DateTime() 
		{
		}

		public DateTime(DateTime obj)
		{
			myValue = obj.myValue;
			timezone = obj.timezone;
		}

		public DateTime(System.DateTime newvalue)
		{
			myValue = newvalue;
			timezone = NoTimezone;
		}

		public DateTime(int year, int month, int day, int hour, int minute, double second, int offsetTZ)
		{
			myValue = new System.DateTime(year, month, day, hour, minute, 0);
			decimal ticks = (new System.Decimal(second) * TicksPerSecond);
			myValue = myValue.AddTicks(System.Decimal.ToInt64(ticks));
			timezone = (short)offsetTZ;
		}

		public DateTime(int year, int month, int day, int hour, int minute, double second)
			: this(year,month,day,hour,minute,second,NoTimezone)
		{
		}

		public DateTime(int year, int month, int day)
			: this(year,month,day,0,0,0,NoTimezone)
		{
		}


		#endregion //Constructors

		#region Get, Set
		public System.DateTime Value
		{
			get
			{
				return myValue;
			}
			set
			{
				myValue = value;
			}
		}

		public bool HasTimezone
		{
			get
			{
				return timezone != NoTimezone;
			}
		}

		public short TimezoneOffset
		{
			get
			{
				return timezone;
			}
			set
			{
				timezone = value;
			}
		}
		#endregion //Get, Set

		#region Utility functions

		string doTimezone()
		{
			short timezone = this.timezone;
			if (timezone != NoTimezone)
			{
				if (timezone == 0)
					return "Z";

				string r = "+";
				if (timezone < 0)
				{
					timezone = (short)-timezone;
					r = "-";
				}

				int h = timezone / 60;
				int m = timezone % 60;
				r += h.ToString("00") + ":" + m.ToString("00");
				return r;
			}
			return "";
		}

		string killFraction(string s)
		{
			return s.TrimEnd('0').TrimEnd('.');
		}


		public string ToString(DateTimeFormat format)
		{
			switch (format)
			{
				case DateTimeFormat.W3_dateTime:
					return killFraction(myValue.ToString("yyyy-MM-ddTHH\\:mm\\:ss.fffffff", System.Globalization.CultureInfo.InvariantCulture)) + doTimezone();

				case DateTimeFormat.W3_date:
					return myValue.ToString("yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();

				case DateTimeFormat.W3_time:
					return killFraction(myValue.ToString("HH\\:mm\\:ss.fffffff", System.Globalization.CultureInfo.InvariantCulture)) + doTimezone();

				case DateTimeFormat.W3_gYear:
					return myValue.ToString("yyyy", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();

				case DateTimeFormat.W3_gYearMonth:
					return myValue.ToString("yyyy-MM", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();

				case DateTimeFormat.W3_gMonth:
					return myValue.ToString("--MM", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();

				case DateTimeFormat.W3_gMonthDay:
					return myValue.ToString("--MM-dd", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();
				
				case DateTimeFormat.W3_gDay:
					return myValue.ToString("---dd", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();

				case DateTimeFormat.S_DateTime:
					return myValue.ToString("yyyy-MM-dd HH\\:mm\\:ss.fffffff", System.Globalization.CultureInfo.InvariantCulture) + doTimezone();

				case DateTimeFormat.S_Seconds:
				{
					decimal d = myValue.Ticks;
					d /= TicksPerSecond;
					return killFraction(d.ToString());
				}

				case DateTimeFormat.S_Days:
				{
					decimal d = myValue.Ticks;
					d /= 86400 * TicksPerSecond;
					return killFraction(d.ToString());
				}
			}
			throw new ArgumentException("Invalid format.");
		}

		public int GetWeekOfMonth()
		{
			DateTime date = new DateTime(myValue.Year, myValue.Month, 1);
			int[] ww = new int[] { 7, 1, 2, 3, 4, 5, 6 };
			int weekday1st = ww[(int)date.Value.DayOfWeek]; // Mon:1 ... Sun:7
			int dayInMonth = myValue.Day;
			int dayOfFirstMonday = 0;
			int firstDayFirstWeek = 1;
			int weekIncrement = 1;
			if (weekday1st > 4)
			{
				firstDayFirstWeek += 8 - weekday1st;
				dayOfFirstMonday = firstDayFirstWeek;
				if (dayInMonth < dayOfFirstMonday)
					return 0;
			}
			else
			{
				weekIncrement++;
				dayOfFirstMonday = firstDayFirstWeek + 8 - weekday1st;
			}
			if (dayInMonth < dayOfFirstMonday)
				return 1;
			
			return (int)(Math.Ceiling((dayInMonth - dayOfFirstMonday) / 7m)-1) + weekIncrement;
		}

		public System.DateTime GetDateTime(bool correctTZ) 
		{
			System.DateTime result = myValue;
			if( correctTZ && timezone != NoTimezone )
				result.AddMinutes( -timezone );
			return result;
		}

		#endregion //Utility functions

		public static readonly long TicksPerSecond = 10000000;
		
		public static DateTime Parse( string s )
		{
			string newvalue = s.Trim(); 
			
			DateTime dt = new DateTime();
			if (newvalue != null &&
				!dt.ParseDateTime(newvalue, DateTimePart.Date|DateTimePart.Time) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Date) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Time) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Year|DateTimePart.Month) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Month | DateTimePart.Day) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Year) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Month ) &&
				!dt.ParseDateTime(newvalue, DateTimePart.Day))
						throw new StringParseException(newvalue + " cannot be converted to a dateTime value");
			
			return dt;
		}


		public override string ToString()
		{
			return ToString(DateTimeFormat.W3_dateTime);
		}

		public static DateTime Now
		{
			get
			{
				System.DateTime dt = System.DateTime.Now;
				System.TimeSpan tzofs = System.TimeZoneInfo.Local.GetUtcOffset( dt );
				DateTime ret = new DateTime( dt );
				ret.TimezoneOffset = (short) tzofs.TotalMinutes;
				return ret;
			}
		}


		public static bool operator==(DateTime a, DateTime b)
		{
			if (Object.ReferenceEquals(a,null))
				return Object.ReferenceEquals(b,null);
			return a.Equals(b);
		}

		public static bool operator!=(DateTime a, DateTime b)
		{
			return !(a == b);
		}

		public static bool operator<(DateTime a, DateTime b)
		{
			if (Object.ReferenceEquals(a,null))
				return !Object.ReferenceEquals(b,null);
			if (Object.ReferenceEquals(b,null))
				return false;
			return a.CompareTo(b) < 0;
		}
		
		public static bool operator>=(DateTime a, DateTime b)
		{
			return !(a < b);
		}

		public static bool operator<=(DateTime a, DateTime b)
		{
			return !(b < a);
		}

		public static bool operator>(DateTime a, DateTime b)
		{
			return b < a;
		}
	}
	
	public class Duration
	{
		protected System.TimeSpan myValue = new System.TimeSpan(0);
		protected int months = 0;
		protected int years = 0;

		public enum ParseType { DURATION, YEARMONTH, DAYTIME };

		#region Constructors
		public Duration()
		{
		}

		public Duration(Duration obj)
		{
			myValue = obj.myValue;
			months = obj.months;
			years = obj.years;
		}

		public Duration(System.TimeSpan newvalue)
		{
			myValue = newvalue;
			/*
			myValue = new System.TimeSpan(newvalue.Days % 31, newvalue.Hours, newvalue.Minutes, newvalue.Seconds, newvalue.Milliseconds);
			months = (newvalue.Days / 31) % 12;
			years = newvalue.Days / 31 / 12;
			*/
		}

		public Duration(long ticks)
		{
			myValue = new System.TimeSpan( ticks );
		}

		public Duration(int newyears, int newmonths, int days, int hours, int minutes, int seconds, double partseconds, bool bnegative)
		{
			years = newyears;
			months = newmonths;
			myValue = new System.TimeSpan( days, hours, minutes, seconds, (int)(0) );
			myValue += new System.TimeSpan((long)(partseconds * System.TimeSpan.TicksPerSecond));
			if( bnegative )
			{
				myValue = -myValue;
				years = -years;
				months = -months;
			}
		}

	#endregion //Constructors

		#region Get, Set
		public System.TimeSpan Value
		{
			get
			{
				return myValue;
			}
			set
			{
				myValue = value;
			}
		}
		public int Years
		{
			get
			{
				return years;
			}
			set
			{
				years = value;
			}
		}
		public int Months
		{
			get
			{
				return months;
			}
			set
			{
				months = value;
			}
		}

		public static Duration Parse( string s, ParseType pt )
		{
			string newvalue = s.Trim();
			
			Duration d = new Duration();
			if ( newvalue == null || newvalue.Length == 0 )
				throw new StringParseException("Duration must start with P or -P followed by a duration value.");
			
			int pos = 0;
			bool bNegative = false;
			int day = 0;
			int hour = 0;
			int minute = 0;
			int second = 0;
			double partsecond = 0.0;
			d.months = 0;	
			d.years = 0;
			
			if (newvalue[pos] == '-') 
			{
				bNegative = true;
				++pos;
			}

			if (pos == newvalue.Length || newvalue[pos] != 'P') 
				throw new StringParseException("Duration must start with P or -P followed by a duration value.");
			++pos;

			int state = 0;	// year component
			while ( pos != newvalue.Length )
			{
				// no more data allowed?
				if (state == 8) 
					throw new StringParseException("Extra data after duration value.");
						
				// check if ymd part is over
				if (newvalue[pos] == 'T') 
				{
					if (state >= 4) // hour
						throw new StringParseException("Multiple Ts inside duration value.");
					state = 4;
					++pos;			
				}

				if (state == 3) // 'T' separator
					throw new StringParseException("Extra data after duration value.");

				// now a digit has to follow, and probably more than one
				if (pos == newvalue.Length || newvalue[pos] < '0' || newvalue[pos] > '9') 
					throw new StringParseException("Invalid numeric data in duration value.");

				int val = 0;
				int digits = 0;
				while (pos != newvalue.Length && newvalue[pos] >= '0' && newvalue[pos] <= '9')
				{
					if (val >= 100000000) 
						throw new StringParseException("Numeric overflow in duration value.");
					val = val * 10 + (newvalue[pos] - '0');
					digits += 1;
					++pos;
				}

				if (pos == newvalue.Length) 
					throw new StringParseException("Duration value missing component designator.");
				
				int foundState = 8;	// bad
				switch (newvalue[pos]) 
				{
				case 'Y': if (state >= 4) foundState = 8; else foundState = 0; break;
				case 'M': if (state >= 4) foundState = 5; else foundState = 1; break;
				case 'D': if (state >= 4) foundState = 8; else foundState = 2; break;
				case 'H': if (state >= 4) foundState = 4; else foundState = 8; break;
				case 'S': if (state >= 7) foundState = 7; else if (state >= 4) foundState = 6; else foundState = 8; break;
				case '.': if (state >= 4) foundState = 6; else foundState = 8; break;
				}

				if (foundState == 8 || foundState < state) 
					throw new StringParseException("Invalid or duplicate component designator.");

				++pos;

				switch ( foundState )
				{
				case 0:
				{
					if ( pt == ParseType.DAYTIME )
						throw new StringParseException("Year not allowed in DayTimeDuration");
					d.years = val;
				}
				break;
				case 1:
				{
					if ( pt == ParseType.DAYTIME )
						throw new StringParseException("Month not allowed in DayTimeDuration");
					d.months = val;
				}
				break;
				case 2:
				{
					if ( pt == ParseType.YEARMONTH )
						throw new StringParseException("Day not allowed in YearMonthDuration");
					day = val;
				}
				break;
				case 4:
				{
					if ( pt == ParseType.YEARMONTH )
						throw new StringParseException("Hour not allowed in YearMonthDuration");
					hour = val;
				}
				break;
				case 5:
				{
					if ( pt == ParseType.YEARMONTH )
						throw new StringParseException("Minute not allowed in YearMonthDuration");
					minute = val;
				}
				break;
				case 6:
				{
					if ( pt == ParseType.YEARMONTH )
						throw new StringParseException("Second not allowed in YearMonthDuration");
					second = val;
				}
				break;
				case 7:
				{
					if ( pt == ParseType.YEARMONTH )
						throw new StringParseException("Millisecond not allowed in YearMonthDuration");
					partsecond = val * System.Math.Pow(0.1, digits);
				}
				break;
				}
		
				state = foundState + 1;
			}					
			if (state == 0) 
				throw new StringParseException("No components given after P in duration value.");

			d.myValue = new System.TimeSpan(day, hour, minute, second, (int)(0));
			d.myValue += new System.TimeSpan((long)(partsecond * System.TimeSpan.TicksPerSecond));
			if (bNegative) 
			{
				d.myValue = -d.myValue;
				d.years = -d.years;
				d.months = -d.months;
			}
			return d;
		}
		#endregion // Get, Set

		public override bool Equals(object other)
		{
			Duration dur = other as Duration;
			if (object.ReferenceEquals(dur, null))
				return false;

			return dur.years == years && dur.months == months && dur.myValue == myValue;
		}

		public override int GetHashCode()
		{
			return years.GetHashCode() ^ months.GetHashCode() ^ myValue.GetHashCode();
		}

		public static bool operator==(Duration a, Duration b)
		{
			if (Object.ReferenceEquals(a, null))
				return Object.ReferenceEquals(b, null);

			return a.Equals(b);
		}

		public static bool operator!=(Duration a, Duration b)
		{
			return !(a == b);
		}

		public string ToYearMonthString()
		{
			string s = "";
			if (IsNegative())
				s += "-";
			s += "P";
			int yearmonth_abs_value = System.Math.Abs(years * 12 + months);
			int y = yearmonth_abs_value / 12;
			int m = yearmonth_abs_value % 12;
			if (y != 0) 
				s += y.ToString() + "Y";
			if (m != 0) 
				s += m.ToString() + "M";
			if (s[s.Length-1] == 'P')
				s += "0M";			
			return s;
		}

		public override string ToString()
		{
			string s = "";
			if (IsNegative())
				s += "-";
			s += "P";
			int yearmonth_abs_value = System.Math.Abs(years * 12 + months);
			int y = yearmonth_abs_value / 12;
			int m = yearmonth_abs_value % 12;
			if (y != 0) 
				s += y.ToString() + "Y";
			if (m != 0) 
				s += m.ToString() + "M";
			if (myValue.Days != 0) 
				s += System.Math.Abs(myValue.Days).ToString() + "D";
			double partsecond = (System.Math.Abs(myValue.Ticks) / (double)(System.TimeSpan.TicksPerSecond)) % 1.0;
			if (myValue.Hours!=0 || myValue.Minutes!=0 || myValue.Seconds!=0 || partsecond>0.0 )
			{
				s += "T";
				if (myValue.Hours != 0) 
					s += System.Math.Abs(myValue.Hours).ToString() + "H";
				if (myValue.Minutes != 0) 
					s += System.Math.Abs(myValue.Minutes).ToString() + "M";
				if (myValue.Seconds != 0)
					s += System.Math.Abs(myValue.Seconds).ToString("#0");
				if (partsecond > 0.0 && partsecond < 1.0) 
				{
					if (myValue.Seconds == 0) s += "0";
					partsecond = System.Math.Round(partsecond, 9);
					string sPartSecond = partsecond.ToString("0.##########");
					s += "." + sPartSecond.Substring(2, sPartSecond.Length-2);
				}
				if (myValue.Seconds != 0 || (partsecond > 0 && partsecond < 1))
					s += "S";
			}
			if (s[s.Length-1] == 'P')
				s += "T0S";			
			return s;
		}

		public bool IsNegative()
		{
			int yearmonthvalue = years * 12 + months;
			return yearmonthvalue == 0 ? (myValue.Ticks < 0) : (yearmonthvalue < 0);
		}
	}
	
	public class QName
	{
		private string uri = null;
		private string prefix = null;
		private string local = null;

		
		public QName(string uri, string prefix, string local) { this.uri = uri; this.prefix = prefix; this.local = local; }

		public QName(string u, string l) 
		{ 
			uri = u; 
			int i = l.IndexOf(":");
			if (i == -1)
				local = l; 
			else
			{
				prefix = l.Substring(0, i);
				local = l.Substring(i+1);
			}
		}
		
		public QName(string value) { parse(value); }

		public string Uri { get { return uri; } set { uri = value; } }
		public string Prefix { get { return prefix; } set { prefix = value; } }
		public string LocalName { get { return local; } set { local = value; } }
		public override string ToString() { return prefix == null || prefix.Length == 0 ? local : prefix + ":" + local; }

		private void parse(string value)
		{
			prefix = null;
			uri=null;
			local = value;

			int i = value.IndexOf("{");
			int j = value.IndexOf("}");
			if (i==0 && j>i)
			{
				uri = value.Substring(1, j-1);
				local = value.Substring(j+1);
			}
		}
	}
}
