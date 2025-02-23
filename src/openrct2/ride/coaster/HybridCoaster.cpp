#pragma region Copyright(c) 2014 - 2017 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https//:ggithub.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "../../drawing/Drawing.h"
#include "../../interface/Viewport.h"
#include "../../paint/Paint.h"
#include "../../paint/Supports.h"
#include "../../sprites.h"
#include "../../world/Map.h"
#include "../RideData.h"
#include "../TrackData.h"
#include "../TrackPaint.h"

namespace HybridRC
{
    static uint32_t GetTrackColour(paint_session& session)
    {
        if (session.TrackColours[SCHEME_TRACK] == 0x21600000)
            return 0x21600000; // TODO dirty hack
        else
            return (session.TrackColours[SCHEME_TRACK] & ~0x1F000000)
                | ((session.TrackColours[SCHEME_SUPPORTS] & 0xF80000) << 5);
    }

    static void TrackFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackElement.HasChain())
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_FLAT + direction), 0, 0, 32, 20, 3,
                height, 0, 6, height);
        }
        else
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT + (direction & 1)), 0, 0, 32, 20, 3,
                height, 0, 6, height);
        }
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void TrackStation(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        static constexpr const uint32_t imageIds[4][3] = {
            { (SPR_G2_HYBRID_TRACK_BRAKE + 0), (SPR_G2_HYBRID_TRACK_BLOCK_BRAKE + 0), SPR_STATION_BASE_A_SW_NE },
            { (SPR_G2_HYBRID_TRACK_BRAKE + 1), (SPR_G2_HYBRID_TRACK_BLOCK_BRAKE + 1), SPR_STATION_BASE_A_NW_SE },
            { (SPR_G2_HYBRID_TRACK_BRAKE + 0), (SPR_G2_HYBRID_TRACK_BLOCK_BRAKE + 0), SPR_STATION_BASE_A_SW_NE },
            { (SPR_G2_HYBRID_TRACK_BRAKE + 1), (SPR_G2_HYBRID_TRACK_BLOCK_BRAKE + 1), SPR_STATION_BASE_A_NW_SE },
        };

        if (trackElement.GetTrackType() == TrackElemType::EndStation)
        {
            PaintAddImageAsParentRotated(
                session, direction, imageIds[direction][1] | GetTrackColour(session), 0, 0, 32, 20, 1, height, 0, 6,
                height + 3);
        }
        else
        {
            PaintAddImageAsParentRotated(
                session, direction, imageIds[direction][0] | GetTrackColour(session), 0, 0, 32, 20, 1, height, 0, 6,
                height + 3);
        }

        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);

        track_paint_util_draw_narrow_station_platform(session, ride, direction, height, 10, trackElement);

        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void Track25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackElement.HasChain())
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE + direction + 8), 0, 0, 32, 20,
                2, height, 0, 6, height + 3);
        }
        else
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE + direction + 8), 0, 0, 32, 20, 2,
                height, 0, 6, height + 3);
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void Track60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        const CoordsXYZ boundBoxOffsets[4] = {
            { 0, 6, height },
            { 28, 4, height - 16 },
            { 28, 4, height - 16 },
            { 0, 6, height },
        };
        static const CoordsXYZ boundBoxLengths[4] = {
            { 32, 20, 3 },
            { 2, 24, 93 },
            { 2, 24, 93 },
            { 32, 20, 3 },
        };
        static constexpr const uint32_t imageIds[2][4] = {
            {
                SPR_G2_HYBRID_LIFT_TRACK_STEEP + 12,
                SPR_G2_HYBRID_LIFT_TRACK_STEEP + 13,
                SPR_G2_HYBRID_LIFT_TRACK_STEEP + 14,
                SPR_G2_HYBRID_LIFT_TRACK_STEEP + 15,
            },
            {
                SPR_G2_HYBRID_TRACK_STEEP + 12,
                SPR_G2_HYBRID_TRACK_STEEP + 13,
                SPR_G2_HYBRID_TRACK_STEEP + 14,
                SPR_G2_HYBRID_TRACK_STEEP + 15,
            },
        };

        auto* ps = PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | imageIds[trackElement.HasChain() ? 0 : 1][direction], 0, 0,
            boundBoxLengths[direction].x, boundBoxLengths[direction].y, boundBoxLengths[direction].z, height,
            boundBoxOffsets[direction].x, boundBoxOffsets[direction].y, boundBoxOffsets[direction].z);

        if (direction == 1 || direction == 2)
        {
            session.WoodenSupportsPrependTo = ps;
        }

        wooden_a_supports_paint_setup(session, direction & 1, 21 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 56, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 104, 0x20);
    }

    static void TrackFlatTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackElement.HasChain())
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE + direction), 0, 0, 32, 20, 3,
                height, 0, 6, height);
        }
        else
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE + direction), 0, 0, 32, 20, 3, height,
                0, 6, height);
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void Track25DegUpTo60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackElement.HasChain())
        {
            switch (direction)
            {
                case 0:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 0), 0, 0, 32, 20, 3,
                        height, 0, 6, height);
                    break;
                case 1:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 1), 0, 0, 32, 20, 3,
                        height, 0, 3, height + 3);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 2), 0, 0, 32, 2, 43,
                        height, 0, 28, height);
                    break;
                case 2:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 3), 0, 0, 32, 20, 3,
                        height, 0, 3, height + 3);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 4), 0, 0, 32, 2, 43,
                        height, 0, 28, height);
                    break;
                case 3:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 5), 0, 0, 32, 20, 3,
                        height, 0, 6, height);
                    break;
            }
        }
        else
        {
            switch (direction)
            {
                case 0:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 0), 0, 0, 32, 20, 3, height,
                        0, 6, height + 2);
                    break;
                case 1:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 1), 0, 0, 32, 20, 3, height,
                        0, 6, height);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 2), 0, 0, 32, 2, 43, height,
                        0, 28, height);
                    break;
                case 2:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 3), 0, 0, 32, 20, 3, height,
                        0, 6, height);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 4), 0, 0, 32, 2, 43, height,
                        0, 28, height);
                    break;
                case 3:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 5), 0, 0, 32, 20, 3, height,
                        0, 6, height + 2);
                    break;
            }
        }
        wooden_a_supports_paint_setup(session, direction & 1, 13 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 24, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 72, 0x20);
    }

    static void Track60DegUpTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackElement.HasChain())
        {
            switch (direction)
            {
                case 0:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 6), 0, 0, 32, 20, 3,
                        height, 0, 6, height);
                    break;
                case 1:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 7), 0, 0, 32, 20, 3,
                        height, 0, 6, height);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 8), 0, 0, 32, 1, 66,
                        height, 0, 27, height);
                    break;
                case 2:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 9), 0, 0, 32, 20, 3,
                        height, 0, 6, height);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 10), 0, 0, 32, 1, 66,
                        height, 0, 27, height);
                    break;
                case 3:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP + 11), 0, 0, 32, 20, 3,
                        height, 0, 6, height);
                    break;
            }
        }
        else
        {
            switch (direction)
            {
                case 0:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 6), 0, 0, 32, 20, 3, height,
                        0, 6, height);
                    break;
                case 1:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 7), 0, 0, 32, 20, 3, height,
                        0, 6, height);
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 8), 0, 0, 32, 1, 66, height,
                        0, 27, height);
                    break;
                case 2:
                    session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 9), 0, 0, 32, 20, 3, height,
                        0, 6, height);

                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 10), 0, 0, 32, 1, 66, height,
                        0, 27, height);
                    break;
                case 3:
                    PaintAddImageAsParentRotated(
                        session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP + 11), 0, 0, 32, 20, 3, height,
                        0, 6, height);
                    break;
            }
        }
        wooden_a_supports_paint_setup(session, direction & 1, 17 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 24, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 72, 0x20);
    }

    static void Track25DegUpToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackElement.HasChain())
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE + direction + 4), 0, 0, 32, 20,
                3, height, 0, 6, height);
        }
        else
        {
            PaintAddImageAsParentRotated(
                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE + direction + 4), 0, 0, 32, 20, 2,
                height, 0, 6, height + 3);
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void Track25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track60DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackFlatTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpToFlat(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownTo60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track60DegUpTo25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track60DegDownTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpTo60DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackFlatTo25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track90DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        const CoordsXYZ boundBoxOffsets[4] = {
            { 4, 6, height + 8 },
            { 24, 6, height + 8 },
            { 24, 6, height + 8 },
            { 4, 6, height + 8 },
        };
        static constexpr CoordsXYZ boundBoxLengths[4] = {
            { 2, 20, 31 },
            { 2, 20, 31 },
            { 2, 20, 31 },
            { 2, 20, 31 },
        };
        static constexpr const uint32_t imageIds[4] = {
            SPR_G2_HYBRID_TRACK_VERTICAL + 8,
            SPR_G2_HYBRID_TRACK_VERTICAL + 9,
            SPR_G2_HYBRID_TRACK_VERTICAL + 10,
            SPR_G2_HYBRID_TRACK_VERTICAL + 11,
        };
        switch (trackSequence)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | imageIds[direction], 0, 0, boundBoxLengths[direction].x,
                    boundBoxLengths[direction].y, boundBoxLengths[direction].z, height, boundBoxOffsets[direction].x,
                    boundBoxOffsets[direction].y, boundBoxOffsets[direction].z);
                paint_util_set_vertical_tunnel(session, height + 32);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                break;
        }
    }

    static void Track90DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track90DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track60DegUpTo90DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        const CoordsXYZ boundBoxOffsets[4] = {
            { 0, 6, height },
            { 24, 6, height },
            { 24, 6, height },
            { 0, 6, height },
        };
        static constexpr CoordsXYZ boundBoxLengths[4] = {
            { 32, 20, 3 },
            { 2, 20, 55 },
            { 2, 20, 55 },
            { 32, 20, 3 },
        };
        static constexpr const uint32_t imageIds[4] = {
            SPR_G2_HYBRID_TRACK_VERTICAL + 0,
            SPR_G2_HYBRID_TRACK_VERTICAL + 1,
            SPR_G2_HYBRID_TRACK_VERTICAL + 2,
            SPR_G2_HYBRID_TRACK_VERTICAL + 3,
        };

        switch (trackSequence)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | imageIds[direction], 0, 0, boundBoxLengths[direction].x,
                    boundBoxLengths[direction].y, boundBoxLengths[direction].z, height, boundBoxOffsets[direction].x,
                    boundBoxOffsets[direction].y, boundBoxOffsets[direction].z);
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_vertical_tunnel(session, height + 56);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 1:
                break;
        }
    }

    static void Track90DegDownTo60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track60DegUpTo90DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track90DegUpTo60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        const CoordsXYZ boundBoxOffsets[4] = {
            { 0, 6, height + 8 },
            { 24, 6, height + 8 },
            { 24, 6, height + 8 },
            { 0, 6, height + 8 },
        };
        static constexpr CoordsXYZ boundBoxLengths[4] = {
            { 32, 20, 3 },
            { 2, 20, 31 },
            { 2, 20, 31 },
            { 32, 20, 3 },
        };
        static constexpr const uint32_t imageIds[4] = {
            SPR_G2_HYBRID_TRACK_VERTICAL + 4,
            SPR_G2_HYBRID_TRACK_VERTICAL + 5,
            SPR_G2_HYBRID_TRACK_VERTICAL + 6,
            SPR_G2_HYBRID_TRACK_VERTICAL + 7,
        };

        PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | imageIds[direction], 0, 0, boundBoxLengths[direction].x,
            boundBoxLengths[direction].y, boundBoxLengths[direction].z, height, boundBoxOffsets[direction].x,
            boundBoxOffsets[direction].y, boundBoxOffsets[direction].z);
        switch (direction)
        {
            case 1:
                paint_util_push_tunnel_right(session, height + 48, TUNNEL_SQUARE_8);
                break;
            case 2:
                paint_util_push_tunnel_left(session, height + 48, TUNNEL_SQUARE_8);
                break;
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 80, 0x20);
    }

    static void Track60DegDownTo90DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL + 6), 0, 0, 2, 20, 31,
                            height, 24, 6, height + 8);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL + 7), 0, 0, 32, 20, 3,
                            height, 0, 6, height + 8);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL + 4), 0, 0, 32, 20, 3,
                            height, 0, 6, height + 8);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL + 5), 0, 0, 2, 20, 31,
                            height, 24, 6, height + 8);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height + 48, TUNNEL_SQUARE_8);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 80, 0x20);
                break;
            case 1:
                break;
        }
    }

    static void TrackLeftQuarterTurn3(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 3), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 6), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 9), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 1), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 4), 0, 0, 16, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 7), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 10), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 2), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 5), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 8), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE + 11), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightQuarterTurn3(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackLeftQuarterTurn3(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftQuarterTurn5(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 5), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 10), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 15), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, (direction + 1) & 3), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 1), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 6), 0, 0, 36, 16,
                            3, height, 0, 4, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 11), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 16), 0, 0, 32, 14,
                            3, height, 0, 18, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, (direction + 1) & 3),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 2), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 7), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 12), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 17), 0, 0, 33, 33,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_BC | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4,
                        (direction + 1) & 3),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, (direction + 1) & 3), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 3), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 8), 0, 0, 16, 36,
                            3, height, 4, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 13), 0, 0, 16, 32,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 18), 0, 0, 14, 32,
                            3, height, 18, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, (direction + 1) & 3),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 4), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 9), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 14), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE + 19), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightQuarterTurn5(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackLeftQuarterTurn5(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftEighthToDiag(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 4), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 8), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 12), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 1), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 5), 0, 0, 34, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 9), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 13), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 2), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 6), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 10), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 14), 0, 0, 34, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 3), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 7), 0, 0, 16, 18,
                            3, height, 0, 16 + 8, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 11), 0, 0, 16, 16,
                            3, height, 0, 0, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 15), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightEighthToDiag(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 16), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 20), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 24), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 28), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 17), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 21), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 25), 0, 0, 34, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 29), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 18), 0, 0, 34, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 22), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 26), 0, 0, 28, 28,
                            3, height, 4, 4, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 30), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 19), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 23), 0, 0, 16, 16,
                            3, height, 0, 0, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 27), 0, 0, 16, 18,
                            3, height, 0, 16, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE + 31), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackLeftEighthToOrthogonal(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftEighthTurnToOrthogonal[trackSequence];
        TrackRightEighthToDiag(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightEighthToOrthogonal(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftEighthTurnToOrthogonal[trackSequence];
        TrackLeftEighthToDiag(session, ride, trackSequence, (direction + 3) & 3, height, trackElement);
    }

    static void TrackDiagFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_FLAT_DIAGONAL + 3), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_DIAGONAL + 3), -16, -16,
                                32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_FLAT_DIAGONAL + 0), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_DIAGONAL + 0), -16, -16,
                                32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_FLAT_DIAGONAL + 2), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_DIAGONAL + 2), -16, -16,
                                32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_FLAT_DIAGONAL + 1), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_DIAGONAL + 1), -16, -16,
                                32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackDiag25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 11),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 11), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 8),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 8), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 10),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 10), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 9),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 1), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
        }
    }

    static void TrackDiag25DegUpToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 7),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 7), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 4),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 4), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 6),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 6), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 5),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 5), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
        }
    }

    static void TrackDiagFlatTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 3),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 3), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 0),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 0), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 2),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 2), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 1),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 1), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackDiag25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 9),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 9), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 10),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 10), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 8),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 8), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 11),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 11), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
        }
    }

    static void TrackDiagFlatTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 5),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 5), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 6),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 6), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 4),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 4), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 7),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 7), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
        }

        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackDiag25DegDownToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 1),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 1), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 2),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 2), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 0),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 0), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_GENTLE_DIAGONAL + 3),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_DIAGONAL + 3), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackDiag60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 11),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 11), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 8),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 8), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 10),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 10), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 9),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 9), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
        }
    }

    static void TrackDiag25DegUpTo60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 3),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 3), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 0),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 0), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 2),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 2), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 1),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 1), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackDiag60DegUpTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 7),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 7), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 4),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 4), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 6),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 6), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 5),
                                -16, -16, 16, 16, 3, height, 0, 0, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 5), -16,
                                -16, 16, 16, 3, height, 0, 0, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackDiag60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 9),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 9), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 10),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 10), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 8),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 8), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 11),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 11), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 104, 0x20);
                break;
        }
    }

    static void TrackDiag25DegDownTo60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 5),
                                -16, -16, 16, 16, 3, height, 0, 0, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 5), -16,
                                -16, 16, 16, 3, height, 0, 0, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 6),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 6), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 4),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 4), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 7),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 7), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackDiag60DegDownTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 1),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 3:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 1), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 2),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 0:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 2), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 0),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 2:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 0), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                if (trackElement.HasChain())
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_LIFT_TRACK_STEEP_DIAGONAL + 3),
                                -16, -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                else
                {
                    switch (direction)
                    {
                        case 1:
                            PaintAddImageAsParentRotated(
                                session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_DIAGONAL + 3), -16,
                                -16, 32, 32, 3, height, -16, -16, height);
                            break;
                    }
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackFlatToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 0), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 1), 0, 0, 32, 1, 26,
                    height, 0, 27, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 2), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 3), 0, 0, 32, 1, 26,
                    height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 4), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 5), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void TrackFlatToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 6), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 7), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 8), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 9), 0, 0, 32, 1, 26,
                    height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 10), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 11), 0, 0, 32, 1, 26,
                    height, 0, 27, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void TrackLeftBankToflat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackFlatToRightBank(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBankToflat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackFlatToLeftBank(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackLeftBankTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 12), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 13), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 14), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 15), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 16), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 17), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void TrackRightBankTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 18), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 19), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 20), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 21), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 22), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 23), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void Track25DegUpToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 24), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 25), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 26), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 27), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 28), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 29), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void Track25DegUpToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 30), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 31), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 32), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 33), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 34), 0, 0, 32, 20, 2,
                    height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 35), 0, 0, 32, 1, 34,
                    height, 0, 27, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void TrackLeftBankTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpToRightBank(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBankTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpToLeftBank(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightBankTo25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftBankTo25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackLeftbank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 36), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 37), 0, 0, 32, 1, 26,
                    height, 0, 27, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 38), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 39), 0, 0, 32, 1, 26,
                    height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 40), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION + 41), 0, 0, 32, 20, 3,
                    height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void TrackRightbank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftbank(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackDiagFlatToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 4),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 0),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 1),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 3),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 2),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackDiagFlatToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 9),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 5),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 7),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 8),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 6),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackDiagLeftBankToflat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 6),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 7),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 8),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 5),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 9),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackDiagRightBankToflat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 2),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 3),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 0),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 1),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 4),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackDiagLeftBankTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 14),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 10),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 11),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 35);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 13),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 12),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackDiagRightBankTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 19),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 15),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 17),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 18),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 35);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 16),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackDiag25DegUpToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 24),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 20),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 21),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 36);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 23),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 22),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
        }
    }

    static void TrackDiag25DegUpToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 29),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 25),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 27),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 28),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 36);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 26),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
        }
    }

    static void TrackDiagLeftBankTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 26),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 27),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 28),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 36);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 25),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 29),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
        }

        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackDiagRightBankTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 22),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 23),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 20),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 21),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 36);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height + 16, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 24),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                break;
        }

        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackDiag25DegDownToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 16),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 17),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 18),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 15),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 19),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackDiag25DegDownToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 12),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 13),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 10),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 11),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 14),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackDiagLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 34),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 30),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 31),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 33),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 32),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackDiagRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 32),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 33),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 30),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 31),
                            -16, -16, 32, 32, 0, height, -16, -16, height + 28);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        wooden_b_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_b_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_b_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_b_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BANK_TRANSITION_DIAGONAL + 34),
                            -16, -16, 32, 32, 3, height, -16, -16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackLeftQuarterTurn3Bank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 0), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 1), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 4), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 5), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 9), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 13), 0, 0,
                            26, 32, 3, height, 6, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 2), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 6), 0, 0,
                            16, 16, 1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 10), 0, 0,
                            16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 14), 0, 0,
                            22, 22, 3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 3), 0, 0,
                            32, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 7), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 8), 0, 0, 1,
                            32, 26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 11), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 12), 0, 0,
                            1, 32, 26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_CURVE_BANKED + 15), 0, 0,
                            32, 26, 3, height, 6, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightQuarterTurn3Bank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackLeftQuarterTurn3Bank(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackBankedLeftQuarterTurn5(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 0), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 1), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 6), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 7), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 14), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 20), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, (direction + 1) & 3), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 2), 0, 0,
                            32, 16, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 8), 0, 0,
                            48, 16, 1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 15), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 21), 0, 0,
                            32, 14, 3, height, 0, 18, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, (direction + 1) & 3),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 3), 0, 0,
                            16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 9), 0, 0,
                            38, 38, 3, height, 0, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 10), 0, 0,
                            16, 16, 1, height, 16, 16, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 16), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 22), 0, 0,
                            38, 38, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_BC | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4,
                        (direction + 1) & 3),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, (direction + 1) & 3), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 4), 0, 0,
                            16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 11), 0, 0,
                            16, 48, 1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 17), 0, 0,
                            16, 32, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 23), 0, 0,
                            14, 32, 3, height, 18, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, (direction + 1) & 3),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 5), 0, 0,
                            32, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 12), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 13), 0, 0,
                            1, 32, 26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 18), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 19), 0, 0,
                            1, 32, 26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_MEDIUM_CURVE_BANKED + 24), 0, 0,
                            32, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackBankedRightQuarterTurn5(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackBankedLeftQuarterTurn5(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftEighthBankToDiag(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 0), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 1), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 5), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 6), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 12), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 16), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 2), 0, 0,
                            32, 16, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 7), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 8), 0, 0,
                            34, 16, 0, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 13), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 17), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 3), 0, 0,
                            16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 9), 0, 0,
                            16, 16, 0, height, 16, 16, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 14), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 18), 0, 0,
                            34, 16, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 4), 0, 0,
                            16, 16, 3, height, 16, 16, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 10), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 11), 0, 0,
                            16, 18, 0, height, 0, 16, height + 28);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 15), 0, 0,
                            16, 16, 3, height, 0, 0, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 19), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightEighthBankToDiag(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 20), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 24), 0, 0,
                            32, 32, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 28), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 29), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 35), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 36), 0, 0,
                            32, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 21), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 25), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 30), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 31), 0, 0,
                            34, 16, 0, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 37), 0, 0,
                            32, 16, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 22), 0, 0,
                            34, 16, 3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 26), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 32), 0, 0,
                            28, 28, 0, height, 4, 4, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 38), 0, 0,
                            16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 23), 0, 0,
                            16, 16, 3, height, 16, 0, // TODO
                            height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 27), 0, 0,
                            16, 16, 3, height, 0, 0, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 33), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 34), 0, 0,
                            16, 18, 0, height, 0, 16, height + 28);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_CURVE_BANKED + 39), 0, 0,
                            16, 16, 3, height, 16, 16, height);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackLeftEighthBankToOrthogonal(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftEighthTurnToOrthogonal[trackSequence];
        TrackRightEighthBankToDiag(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightEighthBankToOrthogonal(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftEighthTurnToOrthogonal[trackSequence];
        TrackLeftEighthBankToDiag(session, ride, trackSequence, (direction + 3) & 3, height, trackElement);
    }

    static void TrackLeftQuarterTurn3Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 0), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 2), 0, 6,
                            34, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 4), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 6), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 1), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 3), 6, 0,
                            20, 34, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 5), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 7), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackRightQuarterTurn3Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 8), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 10), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 12), 0, 6,
                            34, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 14), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_C0 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 9), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 11), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 13), 6, 0,
                            20, 34, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE + 15), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackLeftQuarterTurn3Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackRightQuarterTurn3Tile25DegUp(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightQuarterTurn3Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackLeftQuarterTurn3Tile25DegUp(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftQuarterTurn5Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 0), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 5), 0, 0,
                            34, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 10), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 15), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 1), 0, 0,
                            32, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 6), 0, 0,
                            32, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 11), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 16), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 2), 0, 0,
                            16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 7), 0, 0,
                            16, 16, 3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 12), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 17), 0, 0,
                            16, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 3), 0, 0,
                            16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 8), 0, 0,
                            16, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 13), 0, 0,
                            16, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 18), 0, 0,
                            16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 4), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 9), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 14), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 19), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackRightQuarterTurn5Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 20), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 25), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 30), 0, 0,
                            34, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 35), 0, 0,
                            32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 21), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 26), 0, 0,
                            32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 31), 0, 0,
                            32, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 36), 0, 0,
                            32, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 22), 0, 0,
                            16, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 27), 0, 0,
                            16, 16, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 32), 0, 0,
                            16, 16, 3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 37), 0, 0,
                            16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_BC | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 23), 0, 0,
                            16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 28), 0, 0,
                            16, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 33), 0, 0,
                            16, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 38), 0, 0,
                            16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 24), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 29), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 34), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE + 39), 0, 0,
                            20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackLeftQuarterTurn5Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackRightQuarterTurn5Tile25DegUp(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightQuarterTurn5Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackLeftQuarterTurn5Tile25DegUp(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftQuarterTurn1Tile60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 0), 0, 0, 28, 28, 3,
                    height, 2, 2, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 1), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 2), 0, 0, 28, 28, 3,
                    height, 2, 2, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 3), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 4), 0, 0, 28, 3, 48,
                    height, 2, 28, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 5), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 6), 0, 0, 28, 28, 3,
                    height, 2, 2, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 7), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
        }
        track_paint_util_left_quarter_turn_1_tile_tunnel(session, direction, height, -8, TUNNEL_SQUARE_7, +56, TUNNEL_SQUARE_8);
        paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 104, 0x20);
    }

    static void TrackRightQuarterTurn1Tile60DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 8), 0, 0, 28, 28, 3,
                    height, 2, 2, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 9), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 10), 0, 0, 28, 3, 48,
                    height, 2, 28, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 11), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 12), 0, 0, 28, 28, 3,
                    height, 2, 2, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 13), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 14), 0, 0, 28, 28, 3,
                    height, 2, 2, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_STEEP_SMALL_CURVE + 15), 0, 0, 28, 28, 1,
                    height, 2, 2, height + 99);
                wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                break;
        }
        track_paint_util_right_quarter_turn_1_tile_tunnel(
            session, direction, height, -8, TUNNEL_SQUARE_7, +56, TUNNEL_SQUARE_8);
        paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 104, 0x20);
    }

    static void TrackLeftQuarterTurn1Tile60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightQuarterTurn1Tile60DegUp(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightQuarterTurn1Tile60DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftQuarterTurn1Tile60DegUp(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftQuarterTurn1Tile90DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 0), 0, 0, 2, 20,
                            63, height, 4, 6, height + 8);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 1), 0, 0, 2, 20,
                            63, height, 4, 6, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 2), 0, 0, 2, 20,
                            63, height, 24, 6, height + 8);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 3), 0, 0, 2, 20,
                            63, height, 24, 6, height + 8);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 4), 0, 0, 2, 20,
                            63, height, 4, 6, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 5), 0, 0, 2, 20,
                            63, height, 24, 6, height + 8);
                        break;
                }
                paint_util_set_vertical_tunnel(session, height + 96);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 96, 0x20);
                break;
            case 1:
                break;
        }
    }

    static void TrackRightQuarterTurn1Tile90DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 6), 0, 0, 2, 20,
                            63, height, 4, 6, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 7), 0, 0, 2, 20,
                            63, height, 24, 6, height + 8);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 8), 0, 0, 2, 20,
                            63, height, 24, 6, height + 8);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 9), 0, 0, 2, 20,
                            63, height, 4, 6, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 10), 0, 0, 2,
                            20, 63, height, 24, 6, height + 8);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_VERTICAL_TWIST + 11), 0, 0, 2,
                            20, 63, height, 4, 6, height + 8);
                        break;
                }
                paint_util_set_vertical_tunnel(session, height + 96);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 96, 0x20);
                break;
            case 1:
                break;
        }
    }

    static void TrackLeftQuarterTurn1Tile90DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightQuarterTurn1Tile90DegUp(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightQuarterTurn1Tile90DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftQuarterTurn1Tile90DegUp(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void Track25DegUpToLeftBanked25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 0), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 1), 0, 0, 32, 20,
                    2, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 2), 0, 0, 32, 1,
                    34, height, 0, 27, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 3), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 4), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void Track25DegUpToRightBanked25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 5), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 6), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 7), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 8), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 9), 0, 0, 32, 20,
                    2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackLeftBanked25DegUpTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 10), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 11), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 12), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 13), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 14), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackRightBanked25DegUpTo25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 15), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 16), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 17), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 18), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 19), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackLeftBanked25DegDownTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpToRightBanked25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBanked25DegDownTo25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpToLeftBanked25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownToLeftBanked25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightBanked25DegUpTo25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownToRightBanked25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftBanked25DegUpTo25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackLeftBankedFlatToLeftBanked25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 20), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 21), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 22), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 23), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 24), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 25), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void TrackRightBankedFlatToRightBanked25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 26), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 27), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 28), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 29), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 30), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 31), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void TrackLeftBanked25DegUpToLeftBankedFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 32), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 33), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 34), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 35), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 36), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 37), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void TrackRightBanked25DegUpToRightBankedFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 38), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 39), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 40), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 41), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 42), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 43), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void TrackLeftBankedFlatToLeftBanked25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightBanked25DegUpToRightBankedFlat(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBankedFlatToRightBanked25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftBanked25DegUpToLeftBankedFlat(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackLeftBanked25DegDownToLeftBankedFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightBankedFlatToRightBanked25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBanked25DegDownToRightBankedFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftBankedFlatToLeftBanked25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegUpLeftBanked(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 44), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 45), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 46), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 47), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void Track25DegUpRightBanked(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 48), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 49), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 50), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 51), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void Track25DegDownLeftBanked(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpRightBanked(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track25DegDownRightBanked(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track25DegUpLeftBanked(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackFlatToLeftBanked25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 52), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 53), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 54), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 55), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 56), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void TrackFlatToRightBanked25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 57), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 58), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 59), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 60), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 61), 0, 0, 32,
                    20, 3, height, 0, 6, height);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 1 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void TrackLeftBanked25DegUpToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 62), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 63), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 64), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 65), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 66), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void TrackRightBanked25DegUpToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (direction)
        {
            case 0:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 67), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 1:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 68), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
            case 2:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 69), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 70), 0, 0, 32, 1,
                    34, height, 0, 27, height);
                break;
            case 3:
                PaintAddImageAsParentRotated(
                    session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SLOPE_BANK_TRANSITION + 71), 0, 0, 32,
                    20, 2, height, 0, 6, height + 3);
                break;
        }
        wooden_a_supports_paint_setup(session, direction & 1, 5 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_FLAT);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_14);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 40, 0x20);
    }

    static void TrackFlatToLeftBanked25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightBanked25DegUpToFlat(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackFlatToRightBanked25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftBanked25DegUpToFlat(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackLeftBanked25DegDownToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackFlatToRightBanked25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBanked25DegDownToFlat(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackFlatToLeftBanked25DegUp(session, ride, trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackLeftBankedQuarterTurn3Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 0),
                            0, 6, 32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 2),
                            0, 6, 32, 20, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 3),
                            0, 6, 34, 1, 34, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 6),
                            0, 6, 32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 9),
                            0, 6, 32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 1),
                            6, 0, 20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 4),
                            6, 0, 20, 32, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 5),
                            6, 0, 1, 34, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 7),
                            6, 0, 20, 32, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 8),
                            6, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 10),
                            6, 0, 20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackRightBankedQuarterTurn3Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 11),
                            0, 6, 32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 13),
                            0, 6, 32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 16),
                            0, 6, 32, 20, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 17),
                            0, 6, 34, 1, 34, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 20),
                            0, 6, 32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_C0 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 12),
                            6, 0, 20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 14),
                            6, 0, 20, 32, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 15),
                            6, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 18),
                            6, 0, 20, 32, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 19),
                            6, 0, 1, 34, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_SMALL_CURVE_BANKED + 21),
                            6, 0, 20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackLeftBankedQuarterTurn3Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackRightBankedQuarterTurn3Tile25DegUp(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightBankedQuarterTurn3Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackLeftBankedQuarterTurn3Tile25DegUp(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftBankedQuarterTurn5Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 0),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 5),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 6),
                            0, 0, 34, 1, 34, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 15),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 21),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 1),
                            0, 0, 32, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 7),
                            0, 0, 32, 16, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 8),
                            0, 0, 1, 1, 34, height, 30, 30, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 16),
                            0, 0, 32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 22),
                            0, 0, 32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 2),
                            0, 0, 16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 9),
                            0, 0, 16, 16, 3, height, 0, -8, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 10),
                            0, 0, 1, 1, 34, height, 30, 30, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 17),
                            0, 0, 1, 1, 3, height, 64, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 23),
                            0, 0, 16, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 3),
                            0, 0, 16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 11),
                            0, 0, 16, 32, 3, height, -8, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 12),
                            0, 0, 1, 1, 34, height, 30, 30, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 18),
                            0, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 24),
                            0, 0, 16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 4),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 13),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 14),
                            0, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 19),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 20),
                            0, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 25),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackRightBankedQuarterTurn5Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 26),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 31),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 37),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 38),
                            0, 0, 34, 1, 34, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 47),
                            0, 0, 32, 20, 3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 27),
                            0, 0, 32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 32),
                            0, 0, 32, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 39),
                            0, 0, 32, 16, 3, height, 0, 16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 40),
                            0, 0, 1, 1, 34, height, 30, 30, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 48),
                            0, 0, 32, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 28),
                            0, 0, 16, 16, 3, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 33),
                            0, 0, 1, 1, 3, height, 64, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 41),
                            0, 0, 16, 16, 3, height, -8, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 42),
                            0, 0, 1, 1, 34, height, 30, 30, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 49),
                            0, 0, 16, 16, 3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_BC | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 29),
                            0, 0, 16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 34),
                            0, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 43),
                            0, 0, 16, 32, 3, height, -8, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 44),
                            0, 0, 1, 1, 34, height, 30, 30, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 50),
                            0, 0, 16, 32, 3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 30),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 35),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 36),
                            0, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 45),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 46),
                            0, 0, 1, 32, 34, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_GENTLE_MEDIUM_CURVE_BANKED + 51),
                            0, 0, 20, 32, 3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 72, 0x20);
                break;
        }
    }

    static void TrackLeftBankedQuarterTurn5Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackRightBankedQuarterTurn5Tile25DegUp(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightBankedQuarterTurn5Tile25DegDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackLeftBankedQuarterTurn5Tile25DegUp(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackSBendLeft(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 0), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 4), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 3), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 7), 0, 0, 32, 32, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 1), 0, 0, 32, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 5), 0, 0, 34, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 2), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 6), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 2), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 6), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 1), 0, 0, 32, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 5), 0, 0, 34, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 3), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 7), 0, 0, 32, 32, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 0), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 4), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 1:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 2:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackSBendRight(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 8), 0, 0, 32, 32, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 12), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 11), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 15), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 9), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 13), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 10), 0, 0, 34, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 14), 0, 0, 32, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 10), 0, 0, 34, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 14), 0, 0, 32, 26, 3,
                            height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 9), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 13), 0, 0, 32, 26, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 11), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 15), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 8), 0, 0, 32, 32, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_S_BEND + 12), 0, 0, 32, 20, 3,
                            height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 1:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 2:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackLeftHalfBankedHelixUpSmall(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 1), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 4), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 5), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 9), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 13), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 2), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 6), 0, 0, 16, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 10), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 14), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 3), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 7), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 8), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 11), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 12), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 15), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 13), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 0), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 1), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 4), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 5), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 9), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 14), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 2), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 6), 0, 0, 16, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 10), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_C0 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 7:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 15), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 3), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 7), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 8), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 11), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 12), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightHalfBankedHelixUpSmall(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 16), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 19), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 23), 0, 0, 34, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 24), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 28), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 29), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 17), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 20), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 25), 0, 0, 16, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 30), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_C0 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 18), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 21), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 22), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 26), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 27), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 31), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 19), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 23), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 24), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 28), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 29), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 16), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 20), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 25), 0, 0, 16, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 30), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 17), 0, 0, 16, 16,
                            3, height, 16, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 7:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 21), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 22), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 26), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 27), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 31), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_SMALL_HELIX + 18), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackLeftHalfBankedHelixDownSmall(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackSequence >= 4)
        {
            trackSequence -= 4;
            direction = (direction - 1) & 3;
        }
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackRightHalfBankedHelixUpSmall(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightHalfBankedHelixDownSmall(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackSequence >= 4)
        {
            trackSequence -= 4;
            direction = (direction + 1) & 3;
        }
        trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
        TrackLeftHalfBankedHelixUpSmall(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftHalfBankedHelixUpLarge(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 1), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 6), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 7), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 14), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 20), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 2), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 8), 0, 0, 33, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 15), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 21), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 3), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 9), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 10), 0, 0, 16, 16,
                            1, height, 16, 16, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 16), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 22), 0, 0, 34, 34,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D4 | SEGMENT_B4 | SEGMENT_C0 | SEGMENT_C8 | SEGMENT_D0,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 4), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 11), 0, 0, 16, 33,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 17), 0, 0, 16, 32,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 23), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 5), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 12), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 13), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 18), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 19), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 24), 0, 0, 28, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 7:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 20), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 0), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 1), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 6), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 7), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 14), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 8:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 9:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 21), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 2), 0, 0, 16, 32,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 8), 0, 0, 16, 32,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 15), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 10:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 22), 0, 0, 16, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 3), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 9), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 10), 0, 0, 16, 16,
                            1, height, 16, 16, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 16), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_B8 | SEGMENT_BC | SEGMENT_D0 | SEGMENT_D4,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 11:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 12:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 23), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 4), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 11), 0, 0, 32, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 17), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 13:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 24), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 5), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 12), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 13), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 18), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 19), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackRightHalfBankedHelixUpLarge(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 25), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 30), 0, 0, 32, 32,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 36), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 37), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 44), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 45), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 26), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 31), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 38), 0, 0, 33, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 46), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 27), 0, 0, 34, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 32), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 39), 0, 0, 16, 32,
                            3, height, 0, 16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 40), 0, 0, 16, 16,
                            1, height, 16, 16, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 47), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_B8 | SEGMENT_BC | SEGMENT_D0 | SEGMENT_D4,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 4:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 5:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 28), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 33), 0, 0, 16, 32,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 41), 0, 0, 16, 33,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 48), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 6:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 29), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 34), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 35), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 42), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 43), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 49), 0, 0, 32, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 7:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 30), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 36), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 37), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 44), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 45), 0, 0, 1, 32,
                            26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 25), 0, 0, 20, 32,
                            3, height, 6, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 8:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 9:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 31), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 38), 0, 0, 16, 32,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 46), 0, 0, 16, 32,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 26), 0, 0, 16, 32,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0 | SEGMENT_D4, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 10:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 32), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 39), 0, 0, 16, 16,
                            3, height, 0, 16, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 40), 0, 0, 16, 16,
                            1, height, 16, 16, height + 28);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 47), 0, 0, 16, 16,
                            3, height, 16, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 27), 0, 0, 16, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_BC | SEGMENT_C4 | SEGMENT_CC | SEGMENT_D4 | SEGMENT_B4 | SEGMENT_C0 | SEGMENT_C8 | SEGMENT_D0,
                        direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 11:
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_BC | SEGMENT_CC | SEGMENT_D4, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 12:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 33), 0, 0, 32, 16,
                            3, height, 0, 0, height);
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 41), 0, 0, 32, 16,
                            1, height, 0, 0, height + 28);
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 48), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 28), 0, 0, 32, 16,
                            3, height, 0, 16, height);
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session,
                    paint_util_rotate_segments(
                        SEGMENT_B4 | SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0, direction),
                    0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 13:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 34), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 35), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 42), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 43), 0, 0, 32, 1,
                            26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 49), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_LARGE_HELIX + 29), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
        }
    }

    static void TrackLeftHalfBankedHelixDownLarge(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackSequence >= 7)
        {
            trackSequence -= 7;
            direction = (direction - 1) & 3;
        }
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackRightHalfBankedHelixUpLarge(session, ride, trackSequence, (direction + 1) & 3, height, trackElement);
    }

    static void TrackRightHalfBankedHelixDownLarge(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        if (trackSequence >= 7)
        {
            trackSequence -= 7;
            direction = (direction + 1) & 3;
        }
        trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
        TrackLeftHalfBankedHelixUpLarge(session, ride, trackSequence, (direction - 1) & 3, height, trackElement);
    }

    static void TrackLeftBarrelRollUpToDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 1), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 6), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 7), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 12), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 13), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 18), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 19), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 2), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 3), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 8), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 9), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 14), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 15), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 20), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 21), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 4), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 5), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 10), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 11), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 16), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 17), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 22), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 23), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                }
                switch (direction)
                {
                    case 1:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_INVERTED_9);
                        break;
                    case 2:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_INVERTED_9);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackRightBarrelRollUpToDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 24), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 25), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 30), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 31), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 36), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 37), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 42), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 43), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 32, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 26), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 27), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 32), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 33), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 38), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 39), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 44), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 45), 0, 0, 32, 20,
                            0, height, 0, 6, height + 40);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 28), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 29), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 34), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 35), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 40), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 41), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 46), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BARREL_ROLL + 47), 0, 0, 32, 20,
                            0, height, 0, 6, height + 56);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                switch (direction)
                {
                    case 1:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_INVERTED_9);
                        break;
                    case 2:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_INVERTED_9);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackLeftBarrelRollDownToUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackLeftBarrelRollUpToDown(session, ride, 2 - trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackRightBarrelRollDownToUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackRightBarrelRollUpToDown(session, ride, 2 - trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track90DegToInvertedFlatQuarterLoopUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 0), 0, 0, 2, 20,
                            31, height, 4, 6, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 1), 0, 0, 32, 1,
                            48, height, 0, 32, height + 8);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 6), 0, 0, 2, 20,
                            31, height, 26, 4, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 7), 0, 0, 32, 1,
                            48, height, 0, 32, height + 8);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 12), 0, 0, 2, 20,
                            31, height, 26, 4, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 13), 0, 0, 32, 1,
                            48, height, 0, 32, height + 8);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 18), 0, 0, 2, 20,
                            31, height, 4, 6, height + 8);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 19), 0, 0, 32, 1,
                            48, height, 0, 32, height + 8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 88, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 2), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 3), 0, 0, 32, 1,
                            64, height, 0, 32, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 8), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 9), 0, 0, 32, 1,
                            64, height, 0, 32, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 14), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 15), 0, 0, 32, 1,
                            64, height, 0, 32, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 20), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 21), 0, 0, 32, 1,
                            64, height, 0, 32, height);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 4), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 5), 0, 0, 32, 1,
                            32, height, 0, 32, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 10), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 11), 0, 0, 32, 1,
                            32, height, 0, 32, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 16), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 17), 0, 0, 32, 1,
                            32, height, 0, 32, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 22), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_QUARTER_LOOP + 23), 0, 0, 32, 1,
                            32, height, 0, 32, height);
                        break;
                }
                wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height + 16, TUNNEL_0);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
        }
    }

    static void TrackInvertedFlatTo90DegQuarterLoopDown(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track90DegToInvertedFlatQuarterLoopUp(session, ride, 2 - trackSequence, direction, height, trackElement);
    }

    static void Trackbrakes(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BRAKE + (direction & 1)), 0, 0, 32, 20, 3,
            height, 0, 6, height);
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void TrackOnRidePhoto(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        PaintAddImageAsParentRotated(session, direction, IMAGE_TYPE_REMAP | SPR_STATION_BASE_D, 0, 0, 32, 32, 1, height);
        PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT + (direction & 1)), 0, 0, 32, 20, 0, height,
            0, 6, height + 3);
        track_paint_util_onride_photo_paint(session, direction, height + 3, trackElement);
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 48, 0x20);
    }

    static void TrackFlatTo60DegUpLongBase(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 0), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 4), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 8), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 12), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 50 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 1), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 5), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 9), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 13), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 54 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 2), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 6), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 10), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 14), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 58 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 3), 0, 0, 32, 20,
                            3, height, 0, 6, height);
                        break;
                    case 1:
                        session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 7), 0, 0, 2, 24,
                            56, height, 28, 4, height - 16);
                        break;
                    case 2:
                        session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 11), 0, 0, 2, 24,
                            56, height, 28, 4, height - 16);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 15), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 62 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                switch (direction)
                {
                    case 1:
                        paint_util_push_tunnel_right(session, height + 24, TUNNEL_SQUARE_8);
                        break;
                    case 2:
                        paint_util_push_tunnel_left(session, height + 24, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 80, 0x20);
                break;
        }
    }

    static void Track60DegUpToFlatLongBase(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 16), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 1:
                        session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 20), 0, 0, 2, 24,
                            56, height, 28, 4, height - 16);
                        break;
                    case 2:
                        session.WoodenSupportsPrependTo = PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 24), 0, 0, 2, 24,
                            56, height, 28, 4, height - 16);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 28), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 66 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_7);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 80, 0x20);
                break;
            case 1:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 17), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 21), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 25), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 29), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 70 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 80, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 18), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 22), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 26), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 30), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 74 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 56, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 19), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 23), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 27), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_FLAT_TO_STEEP + 31), 0, 0, 32,
                            20, 3, height, 0, 6, height);
                        break;
                }
                wooden_a_supports_paint_setup(
                    session, direction & 1, 78 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
                switch (direction)
                {
                    case 1:
                        paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                    case 2:
                        paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 40, 0x20);
                break;
        }
    }

    static void TrackFlatTo60DegDownLongBase(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        Track60DegUpToFlatLongBase(session, ride, 3 - trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void Track60DegDownToFlatLongBase(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        TrackFlatTo60DegUpLongBase(session, ride, 3 - trackSequence, (direction + 2) & 3, height, trackElement);
    }

    static void TrackBlockBrakes(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BLOCK_BRAKE + (direction & 1)), 0, 0, 32, 20, 3,
            height, 0, 6, height);
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void Trackbooster(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_BOOSTER + (direction & 1)), 0, 0, 32, 20, 3,
            height, 0, 6, height);
        wooden_a_supports_paint_setup(session, direction & 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
        paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 32, 0x20);
    }

    static void Trackpowered_lift(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        PaintAddImageAsParentRotated(
            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_POWERED_LIFT + direction), 0, 0, 32, 20, 3,
            height, 0, 6, height);
        wooden_a_supports_paint_setup(session, direction & 1, 9 + direction, height, session.TrackColours[SCHEME_SUPPORTS]);
        if (direction == 0 || direction == 3)
        {
            paint_util_push_tunnel_rotated(session, direction, height - 8, TUNNEL_SQUARE_7);
        }
        else
        {
            paint_util_push_tunnel_rotated(session, direction, height + 8, TUNNEL_SQUARE_8);
        }
        paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
        paint_util_set_general_support_height(session, height + 56, 0x20);
    }

    static void TrackLeftBankToLeftQuarterTurn3Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 0), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 2), 0, 6,
                            32, 20, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 3), 0, 6,
                            34, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 5), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 7), 0, 6,
                            32, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 1), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 4), 6, 0,
                            20, 34, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 6), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 8), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_8);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
        }
    }

    static void TrackRightBankToRightQuarterTurn3Tile25DegUp(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 9), 0, 6,
                            32, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 11), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 13), 0, 6,
                            32, 20, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 14), 0, 6,
                            34, 1, 26, height, 0, 27, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 16), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_FLAT);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_C0 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 10), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 12), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 15), 6, 0,
                            20, 34, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 17), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_8);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_8);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
        }
    }

    static void TrackLeftQuarterTurn3Tile25DegDownToLeftBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 12), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 15), 0, 6,
                            34, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 17), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 10), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_B8 | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_D0, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 11), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 13), 6, 0,
                            20, 32, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 14), 6, 0,
                            1, 34, 26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 16), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 9), 6, 0,
                            32, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 2:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 3:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
        }
    }

    static void TrackRightQuarterTurn3Tile25DegDownToRightBank(
        paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
        const TrackElement& trackElement)
    {
        switch (trackSequence)
        {
            case 0:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 8), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 1), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 4), 0, 6,
                            34, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 6), 0, 6,
                            32, 20, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                if (direction == 0 || direction == 3)
                {
                    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_SQUARE_8);
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
            case 1:
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 2:
                switch (direction)
                {
                    case 0:
                        wooden_a_supports_paint_setup(session, 4, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        wooden_a_supports_paint_setup(session, 5, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        wooden_a_supports_paint_setup(session, 2, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        wooden_a_supports_paint_setup(session, 3, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                paint_util_set_segment_support_height(
                    session, paint_util_rotate_segments(SEGMENT_C0 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_D4, direction), 0xFFFF,
                    0);
                paint_util_set_general_support_height(session, height + 48, 0x20);
                break;
            case 3:
                switch (direction)
                {
                    case 0:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 7), 6, 0,
                            32, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 1:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 0), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 2:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 2), 6, 0,
                            20, 32, 3, height);
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 3), 6, 0,
                            1, 34, 26, height, 27, 0, height);
                        wooden_a_supports_paint_setup(session, 1, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                    case 3:
                        PaintAddImageAsParentRotated(
                            session, direction, GetTrackColour(session) | (SPR_G2_HYBRID_TRACK_TURN_BANK_TRANSITION + 5), 6, 0,
                            20, 32, 3, height);
                        wooden_a_supports_paint_setup(session, 0, 0, height, session.TrackColours[SCHEME_SUPPORTS]);
                        break;
                }
                switch (direction)
                {
                    case 0:
                        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                    case 1:
                        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
                        break;
                }
                paint_util_set_segment_support_height(session, paint_util_rotate_segments(SEGMENTS_ALL, direction), 0xFFFF, 0);
                paint_util_set_general_support_height(session, height + 64, 0x20);
                break;
        }
    }

    TRACK_PAINT_FUNCTION GetTrackPaintFunction(int32_t trackType)
    {
        switch (trackType)
        {
            case TrackElemType::Flat:
                return TrackFlat;
            case TrackElemType::EndStation:
            case TrackElemType::BeginStation:
            case TrackElemType::MiddleStation:
                return TrackStation;
            case TrackElemType::Up25:
                return Track25DegUp;
            case TrackElemType::Up60:
                return Track60DegUp;
            case TrackElemType::FlatToUp25:
                return TrackFlatTo25DegUp;
            case TrackElemType::Up25ToUp60:
                return Track25DegUpTo60DegUp;
            case TrackElemType::Up60ToUp25:
                return Track60DegUpTo25DegUp;
            case TrackElemType::Up25ToFlat:
                return Track25DegUpToFlat;
            case TrackElemType::Down25:
                return Track25DegDown;
            case TrackElemType::Down60:
                return Track60DegDown;
            case TrackElemType::FlatToDown25:
                return TrackFlatTo25DegDown;
            case TrackElemType::Down25ToDown60:
                return Track25DegDownTo60DegDown;
            case TrackElemType::Down60ToDown25:
                return Track60DegDownTo25DegDown;
            case TrackElemType::Down25ToFlat:
                return Track25DegDownToFlat;
            case TrackElemType::Up90:
                return Track90DegUp;
            case TrackElemType::Down90:
                return Track90DegDown;
            case TrackElemType::Up60ToUp90:
                return Track60DegUpTo90DegUp;
            case TrackElemType::Down90ToDown60:
                return Track90DegDownTo60DegDown;
            case TrackElemType::Up90ToUp60:
                return Track90DegUpTo60DegUp;
            case TrackElemType::Down60ToDown90:
                return Track60DegDownTo90DegDown;
            case TrackElemType::LeftQuarterTurn5Tiles:
                return TrackLeftQuarterTurn5;
            case TrackElemType::RightQuarterTurn5Tiles:
                return TrackRightQuarterTurn5;
            case TrackElemType::FlatToLeftBank:
                return TrackFlatToLeftBank;
            case TrackElemType::FlatToRightBank:
                return TrackFlatToRightBank;
            case TrackElemType::LeftBankToFlat:
                return TrackLeftBankToflat;
            case TrackElemType::RightBankToFlat:
                return TrackRightBankToflat;
            case TrackElemType::BankedLeftQuarterTurn5Tiles:
                return TrackBankedLeftQuarterTurn5;
            case TrackElemType::BankedRightQuarterTurn5Tiles:
                return TrackBankedRightQuarterTurn5;
            case TrackElemType::LeftBankToUp25:
                return TrackLeftBankTo25DegUp;
            case TrackElemType::RightBankToUp25:
                return TrackRightBankTo25DegUp;
            case TrackElemType::Up25ToLeftBank:
                return Track25DegUpToLeftBank;
            case TrackElemType::Up25ToRightBank:
                return Track25DegUpToRightBank;
            case TrackElemType::LeftBankToDown25:
                return TrackLeftBankTo25DegDown;
            case TrackElemType::RightBankToDown25:
                return TrackRightBankTo25DegDown;
            case TrackElemType::Down25ToLeftBank:
                return Track25DegDownToLeftBank;
            case TrackElemType::Down25ToRightBank:
                return Track25DegDownToRightBank;
            case TrackElemType::LeftBank:
                return TrackLeftbank;
            case TrackElemType::RightBank:
                return TrackRightbank;
            case TrackElemType::LeftQuarterTurn5TilesUp25:
                return TrackLeftQuarterTurn5Tile25DegUp;
            case TrackElemType::RightQuarterTurn5TilesUp25:
                return TrackRightQuarterTurn5Tile25DegUp;
            case TrackElemType::LeftQuarterTurn5TilesDown25:
                return TrackLeftQuarterTurn5Tile25DegDown;
            case TrackElemType::RightQuarterTurn5TilesDown25:
                return TrackRightQuarterTurn5Tile25DegDown;
            case TrackElemType::SBendLeft:
                return TrackSBendLeft;
            case TrackElemType::SBendRight:
                return TrackSBendRight;
            case TrackElemType::LeftQuarterTurn3Tiles:
                return TrackLeftQuarterTurn3;
            case TrackElemType::RightQuarterTurn3Tiles:
                return TrackRightQuarterTurn3;
            case TrackElemType::LeftBankedQuarterTurn3Tiles:
                return TrackLeftQuarterTurn3Bank;
            case TrackElemType::RightBankedQuarterTurn3Tiles:
                return TrackRightQuarterTurn3Bank;
            case TrackElemType::LeftQuarterTurn3TilesUp25:
                return TrackLeftQuarterTurn3Tile25DegUp;
            case TrackElemType::RightQuarterTurn3TilesUp25:
                return TrackRightQuarterTurn3Tile25DegUp;
            case TrackElemType::LeftQuarterTurn3TilesDown25:
                return TrackLeftQuarterTurn3Tile25DegDown;
            case TrackElemType::RightQuarterTurn3TilesDown25:
                return TrackRightQuarterTurn3Tile25DegDown;
            case TrackElemType::LeftHalfBankedHelixUpSmall:
                return TrackLeftHalfBankedHelixUpSmall;
            case TrackElemType::RightHalfBankedHelixUpSmall:
                return TrackRightHalfBankedHelixUpSmall;
            case TrackElemType::LeftHalfBankedHelixDownSmall:
                return TrackLeftHalfBankedHelixDownSmall;
            case TrackElemType::RightHalfBankedHelixDownSmall:
                return TrackRightHalfBankedHelixDownSmall;
            case TrackElemType::LeftHalfBankedHelixUpLarge:
                return TrackLeftHalfBankedHelixUpLarge;
            case TrackElemType::RightHalfBankedHelixUpLarge:
                return TrackRightHalfBankedHelixUpLarge;
            case TrackElemType::LeftHalfBankedHelixDownLarge:
                return TrackLeftHalfBankedHelixDownLarge;
            case TrackElemType::RightHalfBankedHelixDownLarge:
                return TrackRightHalfBankedHelixDownLarge;
            case TrackElemType::LeftQuarterTurn1TileUp60:
                return TrackLeftQuarterTurn1Tile60DegUp;
            case TrackElemType::RightQuarterTurn1TileUp60:
                return TrackRightQuarterTurn1Tile60DegUp;
            case TrackElemType::LeftQuarterTurn1TileDown60:
                return TrackLeftQuarterTurn1Tile60DegDown;
            case TrackElemType::RightQuarterTurn1TileDown60:
                return TrackRightQuarterTurn1Tile60DegDown;
            case TrackElemType::Brakes:
                return Trackbrakes;
            case TrackElemType::Up25LeftBanked:
                return Track25DegUpLeftBanked;
            case TrackElemType::Up25RightBanked:
                return Track25DegUpRightBanked;
            case TrackElemType::OnRidePhoto:
                return TrackOnRidePhoto;
            case TrackElemType::Down25LeftBanked:
                return Track25DegDownLeftBanked;
            case TrackElemType::Down25RightBanked:
                return Track25DegDownRightBanked;
            case TrackElemType::FlatToUp60LongBase:
                return TrackFlatTo60DegUpLongBase;
            case TrackElemType::Up60ToFlatLongBase:
                return Track60DegUpToFlatLongBase;
            case TrackElemType::FlatToDown60LongBase:
                return TrackFlatTo60DegDownLongBase;
            case TrackElemType::Down60ToFlatLongBase:
                return Track60DegDownToFlatLongBase;
            case TrackElemType::LeftEighthToDiag:
                return TrackLeftEighthToDiag;
            case TrackElemType::RightEighthToDiag:
                return TrackRightEighthToDiag;
            case TrackElemType::LeftEighthToOrthogonal:
                return TrackLeftEighthToOrthogonal;
            case TrackElemType::RightEighthToOrthogonal:
                return TrackRightEighthToOrthogonal;
            case TrackElemType::LeftEighthBankToDiag:
                return TrackLeftEighthBankToDiag;
            case TrackElemType::RightEighthBankToDiag:
                return TrackRightEighthBankToDiag;
            case TrackElemType::LeftEighthBankToOrthogonal:
                return TrackLeftEighthBankToOrthogonal;
            case TrackElemType::RightEighthBankToOrthogonal:
                return TrackRightEighthBankToOrthogonal;
            case TrackElemType::DiagFlat:
                return TrackDiagFlat;
            case TrackElemType::DiagUp25:
                return TrackDiag25DegUp;
            case TrackElemType::DiagUp60:
                return TrackDiag60DegUp;
            case TrackElemType::DiagFlatToUp25:
                return TrackDiagFlatTo25DegUp;
            case TrackElemType::DiagUp25ToUp60:
                return TrackDiag25DegUpTo60DegUp;
            case TrackElemType::DiagUp60ToUp25:
                return TrackDiag60DegUpTo25DegUp;
            case TrackElemType::DiagUp25ToFlat:
                return TrackDiag25DegUpToFlat;
            case TrackElemType::DiagDown25:
                return TrackDiag25DegDown;
            case TrackElemType::DiagDown60:
                return TrackDiag60DegDown;
            case TrackElemType::DiagFlatToDown25:
                return TrackDiagFlatTo25DegDown;
            case TrackElemType::DiagDown25ToDown60:
                return TrackDiag25DegDownTo60DegDown;
            case TrackElemType::DiagDown60ToDown25:
                return TrackDiag60DegDownTo25DegDown;
            case TrackElemType::DiagDown25ToFlat:
                return TrackDiag25DegDownToFlat;
            case TrackElemType::DiagFlatToLeftBank:
                return TrackDiagFlatToLeftBank;
            case TrackElemType::DiagFlatToRightBank:
                return TrackDiagFlatToRightBank;
            case TrackElemType::DiagLeftBankToFlat:
                return TrackDiagLeftBankToflat;
            case TrackElemType::DiagRightBankToFlat:
                return TrackDiagRightBankToflat;
            case TrackElemType::DiagLeftBankToUp25:
                return TrackDiagLeftBankTo25DegUp;
            case TrackElemType::DiagRightBankToUp25:
                return TrackDiagRightBankTo25DegUp;
            case TrackElemType::DiagUp25ToLeftBank:
                return TrackDiag25DegUpToLeftBank;
            case TrackElemType::DiagUp25ToRightBank:
                return TrackDiag25DegUpToRightBank;
            case TrackElemType::DiagLeftBankToDown25:
                return TrackDiagLeftBankTo25DegDown;
            case TrackElemType::DiagRightBankToDown25:
                return TrackDiagRightBankTo25DegDown;
            case TrackElemType::DiagDown25ToLeftBank:
                return TrackDiag25DegDownToLeftBank;
            case TrackElemType::DiagDown25ToRightBank:
                return TrackDiag25DegDownToRightBank;
            case TrackElemType::DiagLeftBank:
                return TrackDiagLeftBank;
            case TrackElemType::DiagRightBank:
                return TrackDiagRightBank;
            case TrackElemType::BlockBrakes:
                return TrackBlockBrakes;
            case TrackElemType::LeftBankedQuarterTurn3TileUp25:
                return TrackLeftBankedQuarterTurn3Tile25DegUp;
            case TrackElemType::RightBankedQuarterTurn3TileUp25:
                return TrackRightBankedQuarterTurn3Tile25DegUp;
            case TrackElemType::LeftBankedQuarterTurn3TileDown25:
                return TrackLeftBankedQuarterTurn3Tile25DegDown;
            case TrackElemType::RightBankedQuarterTurn3TileDown25:
                return TrackRightBankedQuarterTurn3Tile25DegDown;
            case TrackElemType::LeftBankedQuarterTurn5TileUp25:
                return TrackLeftBankedQuarterTurn5Tile25DegUp;
            case TrackElemType::RightBankedQuarterTurn5TileUp25:
                return TrackRightBankedQuarterTurn5Tile25DegUp;
            case TrackElemType::LeftBankedQuarterTurn5TileDown25:
                return TrackLeftBankedQuarterTurn5Tile25DegDown;
            case TrackElemType::RightBankedQuarterTurn5TileDown25:
                return TrackRightBankedQuarterTurn5Tile25DegDown;
            case TrackElemType::Up25ToLeftBankedUp25:
                return Track25DegUpToLeftBanked25DegUp;
            case TrackElemType::Up25ToRightBankedUp25:
                return Track25DegUpToRightBanked25DegUp;
            case TrackElemType::LeftBankedUp25ToUp25:
                return TrackLeftBanked25DegUpTo25DegUp;
            case TrackElemType::RightBankedUp25ToUp25:
                return TrackRightBanked25DegUpTo25DegUp;
            case TrackElemType::Down25ToLeftBankedDown25:
                return Track25DegDownToLeftBanked25DegDown;
            case TrackElemType::Down25ToRightBankedDown25:
                return Track25DegDownToRightBanked25DegDown;
            case TrackElemType::LeftBankedDown25ToDown25:
                return TrackLeftBanked25DegDownTo25DegDown;
            case TrackElemType::RightBankedDown25ToDown25:
                return TrackRightBanked25DegDownTo25DegDown;
            case TrackElemType::LeftBankedFlatToLeftBankedUp25:
                return TrackLeftBankedFlatToLeftBanked25DegUp;
            case TrackElemType::RightBankedFlatToRightBankedUp25:
                return TrackRightBankedFlatToRightBanked25DegUp;
            case TrackElemType::LeftBankedUp25ToLeftBankedFlat:
                return TrackLeftBanked25DegUpToLeftBankedFlat;
            case TrackElemType::RightBankedUp25ToRightBankedFlat:
                return TrackRightBanked25DegUpToRightBankedFlat;
            case TrackElemType::LeftBankedFlatToLeftBankedDown25:
                return TrackLeftBankedFlatToLeftBanked25DegDown;
            case TrackElemType::RightBankedFlatToRightBankedDown25:
                return TrackRightBankedFlatToRightBanked25DegDown;
            case TrackElemType::LeftBankedDown25ToLeftBankedFlat:
                return TrackLeftBanked25DegDownToLeftBankedFlat;
            case TrackElemType::RightBankedDown25ToRightBankedFlat:
                return TrackRightBanked25DegDownToRightBankedFlat;
            case TrackElemType::FlatToLeftBankedUp25:
                return TrackFlatToLeftBanked25DegUp;
            case TrackElemType::FlatToRightBankedUp25:
                return TrackFlatToRightBanked25DegUp;
            case TrackElemType::LeftBankedUp25ToFlat:
                return TrackLeftBanked25DegUpToFlat;
            case TrackElemType::RightBankedUp25ToFlat:
                return TrackRightBanked25DegUpToFlat;
            case TrackElemType::FlatToLeftBankedDown25:
                return TrackFlatToLeftBanked25DegDown;
            case TrackElemType::FlatToRightBankedDown25:
                return TrackFlatToRightBanked25DegDown;
            case TrackElemType::LeftBankedDown25ToFlat:
                return TrackLeftBanked25DegDownToFlat;
            case TrackElemType::RightBankedDown25ToFlat:
                return TrackRightBanked25DegDownToFlat;
            case TrackElemType::LeftBankToLeftQuarterTurn3TilesUp25:
                return TrackLeftBankToLeftQuarterTurn3Tile25DegUp;
            case TrackElemType::RightBankToRightQuarterTurn3TilesUp25:
                return TrackRightBankToRightQuarterTurn3Tile25DegUp;
            case TrackElemType::LeftQuarterTurn3TilesDown25ToLeftBank:
                return TrackLeftQuarterTurn3Tile25DegDownToLeftBank;
            case TrackElemType::RightQuarterTurn3TilesDown25ToRightBank:
                return TrackRightQuarterTurn3Tile25DegDownToRightBank;
            case TrackElemType::LeftQuarterTurn1TileUp90:
                return TrackLeftQuarterTurn1Tile90DegUp;
            case TrackElemType::RightQuarterTurn1TileUp90:
                return TrackRightQuarterTurn1Tile90DegUp;
            case TrackElemType::LeftQuarterTurn1TileDown90:
                return TrackLeftQuarterTurn1Tile90DegDown;
            case TrackElemType::RightQuarterTurn1TileDown90:
                return TrackRightQuarterTurn1Tile90DegDown;
            case TrackElemType::LeftBarrelRollUpToDown:
                return TrackLeftBarrelRollUpToDown;
            case TrackElemType::RightBarrelRollUpToDown:
                return TrackRightBarrelRollUpToDown;
            case TrackElemType::LeftBarrelRollDownToUp:
                return TrackLeftBarrelRollDownToUp;
            case TrackElemType::RightBarrelRollDownToUp:
                return TrackRightBarrelRollDownToUp;
            case TrackElemType::Up90ToInvertedFlatQuarterLoop:
                return Track90DegToInvertedFlatQuarterLoopUp;
            case TrackElemType::InvertedFlatToDown90QuarterLoop:
                return TrackInvertedFlatTo90DegQuarterLoopDown;
            case TrackElemType::PoweredLift:
                return Trackpowered_lift;
            case TrackElemType::Booster:
                return Trackbooster;
        }
        return nullptr;
    }
} // namespace HybridRC
